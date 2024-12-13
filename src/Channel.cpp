#include "Channel.hpp"

//IN SERVER WE NEED TO CHECK IF THE NAME IS EMPTY AFTER THE CREATION OF THE CHANNEL BECAUSE IF YES THEN SEND MSG TO THE USER
//Maybe throw std::invalid_argument if name of the channel is wrong
Channel::Channel(std::string& name, Logger& logger) : name(name), topic(""), logger(logger), topic_restricted(false), invite_only(false), is_protected(false)
{
    if (name.empty() || name.length() > 200 || 
        (name[0] != '#' && name[0] != '&') ||
        name.find(' ') != std::string::npos || 
        name.find('\a') != std::string::npos || 
        name.find(',') != std::string::npos)
    {
        logger.log(ERROR, "Invalid channel name: " + name);
        this->name = ""; 
        return;
    }
    logger.log(INFO, "Channel created: " + name);
}

Channel::~Channel() 
{
    logger.log(INFO, "Channel destroyed: " + name);
}

//Also add channel to the user container "std::vector<std::string> channels;""
void Channel::addUser(User& user, const std::string& password = "") 
{
    if (isProtected() && password != this->password)
        throw (std::invalid_argument("Wrong password for channel " + name + ": " + password)); 
    if (!invite_only)
    {
        members.insert(std::make_pair(user.getSocketFd(), user));
        if (members.size() == 1) 
        {
            operators.insert(std::make_pair(user.getSocketFd(), user));
            logger.log(INFO, user.getNickname() + " is the operator of the channel " + name);
        }
        broadcast(user.getNickname() + " joined channel " + name);
    }
    else
    {
        if (invited.find(user.getSocketFd()) != invited.end())
        {
            members.insert(std::make_pair(user.getSocketFd(), user));
            invited.erase(user.getSocketFd());
            broadcast(user.getNickname() + " joined channel " + name);
        }
        else
        {
            user.sendMessage(name + "channel is invite-only");
            logger.log(WARNING, user.getNickname() + " tried to join channel " + name + " without invitation");
        }
    }
}

void Channel::removeUser(User& user, std::string& reason) 
{
    members.erase(user.getSocketFd());
    operators.erase(user.getSocketFd());
    logger.log(INFO, user.getNickname() + " left channel " + name);
}

void Channel::setTopic(const std::string& topic, User& user) 
{
    if (topic_restricted && !is_operator(user))
    {
        logger.log(WARNING, user.getNickname() + " tried to set the topic for channel " + name + " without permission.");
        user.sendMessage("482 " + user.getNickname() + " :You're not a channel operator");
        return;
    }
    if (topic.length() > 512) 
    {
        logger.log(WARNING, user.getNickname() + " tried to set an overly long topic for channel " + name);
        user.sendMessage("413 " + user.getNickname() + " :Topic is too long");
        return;
    }
    this->topic = topic;
    broadcast(":" + user.getNickname() + " TOPIC " + name + " :" + topic);
}

std::string Channel::getName() const
{
    return (this->name);
}

std::string Channel::getTopic() const
{
    return (this->topic);
}

std::map<int, User> Channel::getMembers() const
{
    return (members);
}

bool Channel::is_operator(User& user)
{
    if (operators.find(user.getSocketFd()) != operators.end())
        return (true);
    return (false);
}

void Channel::kickUser(User& operator_user, std::string& tar_user, std::string& reason) 
{
    std::map<int, User>::iterator target_it = members.end();
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it)
    {
        if (it->second.getNickname() == tar_user){
            target_it = it;
            break;
        }
    }
    if (target_it == members.end()){
        operator_user.sendMessage("441 " + tar_user + " " + name + " :They aren't on that channel");
        throw (std::invalid_argument("441 " + tar_user + " " + name + " :They aren't on that channel"));
    }
    User& target_user = target_it->second;
    if (!is_operator(operator_user)) {
        logger.log(WARNING, operator_user.getNickname() + " tried to kick " + target_user.getNickname() + " without operator privileges.");
        operator_user.sendMessage("482 " + operator_user.getNickname() + " :You're not a channel operator");
        return;
    }
    if (members.find(target_user.getSocketFd()) == members.end()) {
        logger.log(WARNING, operator_user.getNickname() + " tried to kick a user who is not in the channel.");
        operator_user.sendMessage("441 " + target_user.getNickname() + " " + name + " :They aren't on that channel");
        return;
    }
    if (is_operator(target_user))
        throw(std::invalid_argument("Operator " + operator_user.getNickname() + " tried to kick another operator " + target_user.getNickname()));
    removeUser(target_user, reason);
    broadcast(":" + operator_user.getNickname() + " KICK " + name + " " + target_user.getNickname() + " :" + reason);
    target_user.sendMessage(":" + operator_user.getNickname() + " KICK " + name + " :" + reason);
}

bool Channel::is_member(User& user)
{
    if (members.find(user.getSocketFd()) != members.end())
        return (true);
    return (false);
}

void Channel::inviteUser(User& operator_user, std::string& tar_user) 
{
    std::map<int, User>::iterator target_it = members.end();
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it)
    {
        if (it->second.getNickname() == tar_user){
            target_it = it;
            break;
        }
    }
    if (target_it == members.end()){
        operator_user.sendMessage("441 " + tar_user + " " + name + " :They aren't on that channel");
        throw (std::invalid_argument("441 " + tar_user + " " + name + " :They aren't on that channel"));
    }
    User& target_user = target_it->second;
    if (!is_member(operator_user) || (!invite_only && !is_operator(operator_user))) 
    {
        logger.log(WARNING, operator_user.getNickname() + " tried to invite " + target_user.getNickname() + " without operator privileges.");
        operator_user.sendMessage("482 " + operator_user.getNickname() + " :You're not a channel operator");
        return;
    }
    if (members.find(target_user.getSocketFd()) != members.end()) {
        logger.log(WARNING, operator_user.getNickname() + " tried to invite " + target_user.getNickname() + " who is already in the channel.");
        operator_user.sendMessage("443 " + target_user.getNickname() + " " + name + " :is already on channel");
        return;
    }
    invited.insert(std::make_pair(target_user.getSocketFd(), target_user));
    target_user.sendMessage(":" + operator_user.getNickname() + " INVITE " + target_user.getNickname() + " :" + name);
    logger.log(INFO, operator_user.getNickname() + " invited " + target_user.getNickname() + " to channel " + name);
}


void Channel::changeMode(User& operator_user, std::vector<std::string> params) 
{
    if (!is_operator(operator_user)) {
        logger.log(WARNING, operator_user.getNickname() + " tried to change channel mode without operator privileges.");
        operator_user.sendMessage("482 " + operator_user.getNickname() + " :You're not a channel operator");
        return;
    }
    if (params.empty() || params[0].length() < 2 || (params[0][0] != '+' && params[0][0] != '-')) {
        operator_user.sendMessage("472 " + operator_user.getNickname() + " :Invalid mode flag");
        return;
    }
    char mode_action = params[0][0];
    char mode = params[0][1];
    bool enable = mode_action == '+';
    std::string mode_param;
    if (params.size() > 1)
        mode_param = params[1];
        else
        mode_param = "";
    switch (mode) 
    {
        case 'i':
            invite_only = enable;
            break;
        case 't':
            topic_restricted = enable;
            break;
        case 'k':
            if (enable) 
            {
                if (mode_param.empty()) 
                {
                    operator_user.sendMessage("461 " + operator_user.getNickname() + " MODE :Not enough parameters");
                    return;
                }
                password = mode_param;
            } 
            else
                password.clear();
            break;
        case 'o':
            if (mode_param.empty()) 
            {
                operator_user.sendMessage("461 " + operator_user.getNickname() + " MODE :Not enough parameters");
                return;
            }
            std::map<int, User>::iterator target_it = members.end();
            for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it)
            {
                if (mode_param == it->second.getNickname()){
                    target_it = it;
                    break;
                }
            }
            if (target_it == members.end()){
                operator_user.sendMessage("441 " + mode_param + " " + name + " :They aren't on that channel");
                return;
            }
            if (enable)
                operators.insert(*target_it);
            if(!enable)
                operators.erase(target_it);
            break;
        default:
            operator_user.sendMessage("472 " + operator_user.getNickname() + " " + mode + " :is unknown mode char");
            return;
    }
    broadcast(":" + operator_user.getNickname() + " MODE " + name + " " + (enable ? "+" : "-") + std::string(1, mode) + " " + mode_param);
}

void Channel::broadcast(std::string msg)
{
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it)
        it->second.sendMessage(msg);
    logger.log(INFO, msg);
}