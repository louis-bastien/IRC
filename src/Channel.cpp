#include "Channel.hpp"
#include "Errors.hpp"


//IN SERVER WE NEED TO CHECK IF THE NAME IS EMPTY AFTER THE CREATION OF THE CHANNEL BECAUSE IF YES THEN SEND MSG TO THE USER
//Maybe throw std::invalid_argument if name of the channel is wrong
Channel::Channel(std::string& name, Logger& logger) : name(name), topic(""), logger(logger), topic_restricted(false), invite_only(false), is_protected(false), user_limit(0)
{
    if (name.empty() || name.length() > 200 || 
        (name[0] != '#' && name[0] != '&') ||
        name.find(' ') != std::string::npos || 
        name.find('\a') != std::string::npos || 
        name.find(',') != std::string::npos)
    {
        logger.log(ERROR, "Invalid channel name: " + name);
        throw std::invalid_argument("Invalid channel name: " + name);
    }
    logger.log(INFO, "Channel created: " + name);
}

Channel::~Channel() 
{
    logger.log(INFO, "Channel destroyed: " + name);
}

//Also add channel to the user container "std::vector<std::string> channels;""
void Channel::addUser(User& user, std::string password) 
{
    if (members.find(user.getSocketFd()) != members.end()) {
            user.sendMessage(ERR_USERONCHANNEL + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + user.getUsername() + " " + name + " :is already on channel");
            throw std::invalid_argument("User already in the channel");
    }
    if (is_protected) {
        if (password.empty()) {
            user.sendMessage(ERR_BADCHANNELKEY + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :Cannot join channel (+k)");
            throw std::invalid_argument("User did not provide password");
        } 
        if (password != this->password) {
            user.sendMessage(ERR_BADCHANNELKEY + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :Cannot join channel (+k)");
            throw std::invalid_argument("Channel password incorrect");
        }
    }
    if (invite_only) {
        if (invited.find(user.getSocketFd()) == invited.end()) {
            user.sendMessage(ERR_INVITEONLYCHAN + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :Cannot join channel (+i)");
            throw std::invalid_argument("User tried to join channel without invitation");
        }
        members.insert(std::make_pair(user.getSocketFd(), user));
        invited.erase(user.getSocketFd());
        broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " JOIN " + name);
        return;
    }
    members.insert(std::make_pair(user.getSocketFd(), user));
    if (members.size() == 1) {
        operators.insert(std::make_pair(user.getSocketFd(), user));
        logger.log(INFO, user.getNickname() + " is the operator of the channel " + name);
    }
    broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " JOIN " + name);
}

void Channel::partUser(User& user, std::string reason = "") 
{
    broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + name + " :" + reason);
    members.erase(user.getSocketFd());
    operators.erase(user.getSocketFd());
    logger.log(INFO, "User " + user.getNickname() + " left channel " + name + (reason.empty() ? "" : " (Reason: " + reason + ")"));
}

void Channel::setTopic(User& user, const std::string& topic) 
{
    if (topic.empty()) {
        if (this->topic.empty())
            user.sendMessage(RPL_NOTOPIC + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :No topic is set");
        else
            user.sendMessage(RPL_TOPIC + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :" + this->topic);
        return;
    }
    if (!is_member(user)) {
        user.sendMessage(ERR_NOTONCHANNEL + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :You're not on that channel");
        throw std::invalid_argument("The user is not part of the channel");
    }
    if (topic_restricted && !is_operator(user)) {
        user.sendMessage(ERR_CHANOPRIVSNEEDED + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :You're not channel operator");
        throw std::invalid_argument("The user is not a channel operator");
    }
    this->topic = topic;
    broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " TOPIC " + name + " :" + topic);
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

bool Channel::isProtected(void) {
    return(is_protected);
}


bool Channel::is_operator(User& user)
{
    if (operators.find(user.getSocketFd()) != operators.end())
        return (true);
    return (false);
}

void Channel::kickUser(User& user, std::string& target, std::string& reason)
{
    if (!is_member(user)) {
        user.sendMessage(ERR_NOTONCHANNEL + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :You're not on that channel");
        throw std::invalid_argument("The user is not part of the channel");
    }

    if (!is_operator(user)) {
        user.sendMessage(ERR_CHANOPRIVSNEEDED + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :You're not channel operator");
        throw std::invalid_argument("The user is not a channel operator");
    }

    std::map<int, User>::iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        if (it->second.getNickname() == target)
            break;
    }
    if (it == members.end()) {
        user.sendMessage(ERR_USERNOTINCHANNEL + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + target + " " + name + " :They aren't on that channel");
        throw std::invalid_argument("The target user is not in the channel");
    }
    User& target_user = it->second;
    broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " KICK " + name + " " + target + " :" + reason);
    members.erase(target_user.getSocketFd());
    operators.erase(target_user.getSocketFd());
}

bool Channel::is_member(User& user)
{
    if (members.find(user.getSocketFd()) != members.end())
        return (true);
    return (false);
}


void Channel::inviteUser(User& user, std::string& target, std::map<int, User>& Users)  {
    if (!is_member(user)) {
        user.sendMessage(ERR_NOTONCHANNEL + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :You're not on that channel");
        throw std::invalid_argument("The user is not part of the channel");
    }
    if (!is_operator(user)) {
        user.sendMessage(ERR_CHANOPRIVSNEEDED + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :You're not channel operator");
        throw std::invalid_argument("The user is not a channel operator");
    }
    std::map<int, User>::iterator it;
    for (it = Users.begin(); it != Users.end(); ++it) {
        if (it->second.getNickname() == target)
            break;
    }
    if (it == Users.end()) {
        user.sendMessage(ERR_NOSUCHNICK + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :No such nick/channel");
        throw std::invalid_argument("The invited user does not exist");
    }
    User& target_user = it->second;
    if (is_member(target_user)) {
        user.sendMessage(ERR_USERONCHANNEL + " " + target_user.getNickname().empty() ? "*" : target_user.getNickname() + " " + name + " :is already on channel");
        throw std::invalid_argument("The invited user is already on the the channel");
    }
    invited.insert(std::make_pair(target_user.getSocketFd(), target_user));
    target_user.sendMessage(RPL_INVITING + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + target + " " + name + " :Invitation successful");
}

void Channel::printMode(User& user) {
    if (!is_member(user)) {
        user.sendMessage(ERR_NOTONCHANNEL + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :You're not on that channel");
        throw std::invalid_argument("The user is not part of the channel");
    }
    std::string modes = "+";
    std::string params;
    if (invite_only)
        modes += "i";
    if (topic_restricted)
        modes += "t";
    if (is_protected) {
        modes += "k";
        params += password;
    }
    if (user_limit != 0) {
        modes += "l";
        params += " " + Utils::toString(user_limit);
    }
    user.sendMessage(RPL_CHANNELMODEIS + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " " + modes + " " + params);
}

void Channel::changeMode(User& user, std::vector<std::string> params) 
{
    if (!is_member(user)) {
        user.sendMessage(ERR_NOTONCHANNEL + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :You're not on that channel");
        throw std::invalid_argument("The user is not part of the channel");
    }
    if (!is_operator(user)) {
        user.sendMessage(ERR_CHANOPRIVSNEEDED + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :You're not channel operator");
        throw std::invalid_argument("The user is not a channel operator");
    }
    if (params[0].length() < 2 || (params[0][0] != '+' && params[0][0] != '-')) {
        user.sendMessage(ERR_UNKNOWNMODE + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :Is unknown mode char");
        throw std::invalid_argument("Channel mode flag(s) incorrect");
    }
    bool enable = params[0][0] == '+';
    int param_index = 1;
    for (int i = 1; i < params[0].size(); i++) {
        char mode = params[0][i];
        switch (mode) 
        {
            case 'i':
                invite_only = enable;
                break;
            case 't':
                topic_restricted = enable;
                break;
            case 'k':
                if (param_index >= params.size()) {
                    user.sendMessage(ERR_NEEDMOREPARAMS +  " " + user.getNickname().empty() ? "*" : user.getNickname() + " MODE :Not enough parameter");
                    throw std::invalid_argument("Missing the new channel password");
                }
                if (enable) {
                    if (is_protected && !password.empty()) {
                        user.sendMessage(ERR_KEYSET +  " " + user.getNickname().empty() ? "*" : user.getNickname() + " :Channel key already set");
                        throw std::invalid_argument("Channel password already set");
                    }
                    password = params[param_index++];
                }
                else
                    password.clear();
                is_protected = enable;
                break;
            case 'o':
                if (param_index > params.size()) {
                    user.sendMessage(ERR_NEEDMOREPARAMS +  " " + user.getNickname().empty() ? "*" : user.getNickname() + " MODE :Not enough parameter");
                    throw std::invalid_argument("Missing the channel operator parameter");
                }
                std::map<int, User>::iterator it;
                for (it = members.begin(); it != members.end(); ++it) {
                    if (params[param_index] == it->second.getNickname())
                        break;
                }
                if (it == members.end()){
                    user.sendMessage(ERR_USERNOTINCHANNEL + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + params[param_index] + " " + name + " :They aren't on that channel");
                    throw std::invalid_argument("The target user is not in the channel");
                }
                if (is_operator(it->second)){
                    logger.log(WARNING, "User " + params[param_index] + " is already an operator on channel " +  name);
                    return;
                }
                if (enable)
                    operators.insert(std::make_pair(user.getSocketFd(), user));
                else
                    operators.erase(operators.find(user.getSocketFd()));
                param_index++;
                break;
            default:
                user.sendMessage(ERR_UNKNOWNMODE + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + name + " :Is unknown mode char");
                throw std::invalid_argument("Channel mode flag(s) incorrect");
        }
    }
    broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " MODE " + name + " " + params[0]);
}

void Channel::broadcast(std::string msg, bool serverPrefix)
{
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it) {
        if (serverPrefix)
            it->second.sendMessage(msg);
        else
            it->second.sendMessage(msg, false);
    }
    logger.log(INFO, msg);
}