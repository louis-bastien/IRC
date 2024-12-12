#include "Channel.hpp"

//IN SERVER WE NEED TO CHECK IF THE NAME IS EMPTY AFTER THE CREATION OF THE CHANNEL BECAUSE IF YES THEN SEND MSG TO THE USER
//Maybe throw std::invalid_argument if name of the channel is wrong
Channel::Channel(std::string& name, Logger& logger) : name(name), topic(""), logger(logger), topic_restricted(false), passPotected(false)
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
    if(passPotected && password != pass_key)
        throw std::invalid_argument("Wrong password for channel " + name + ": " + password);
        
    members.insert(std::make_pair(user.getSocketFd(), user));
    if (members.size() == 1) 
    {
        operators.insert(std::make_pair(user.getSocketFd(), user));
        logger.log(INFO, user.getNickname() + " is the operator of the channel " + name);
    }
    logger.log(INFO, user.getNickname() + " joined channel " + name);
}

void Channel::removeUser(User& user) 
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
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it) 
        it->second.sendMessage(":" + user.getNickname() + " TOPIC " + name + " :" + topic);
    logger.log(INFO, user.getNickname() + " set the topic for channel " + name + " to: " + topic);
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

void Channel::kickUser(User& operator_user, User& target_user, const std::string& reason) 
{
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
    removeUser(target_user);
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it)
        it->second.sendMessage(":" + operator_user.getNickname() + " KICK " + name + " " + target_user.getNickname() + " :" + reason);
    logger.log(INFO, operator_user.getNickname() + " kicked " + target_user.getNickname() + " from channel " + name + " with reason: " + reason);
    target_user.sendMessage(":" + operator_user.getNickname() + " KICK " + name + " :" + reason);
}

void Channel::inviteUser(User& operator_user, User& target_user) 
{
    if (!is_operator(operator_user)) 
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
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it) 
        it->second.sendMessage(":" + operator_user.getNickname() + " MODE " + name + " " + (enable ? "+" : "-") + std::string(1, mode) + " " + mode_param);
    logger.log(INFO, operator_user.getNickname() + " changed channel mode: " + (enable ? "+" : "-") + std::string(1, mode) + " " + mode_param);
}