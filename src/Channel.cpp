#include "Channel.hpp"
#include "Errors.hpp"


//IN SERVER WE NEED TO CHECK IF THE NAME IS EMPTY AFTER THE CREATION OF THE CHANNEL BECAUSE IF YES THEN SEND MSG TO THE USER
//Maybe throw std::invalid_argument if name of the channel is wrong
Channel::Channel(std::string& name, Logger& logger) : name(name), topic(""), logger(logger), topic_restricted(true), invite_only(false), is_protected(false), user_limit(0)
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

Channel::~Channel() {}

//Also add channel to the user container "std::vector<std::string> channels;""
void Channel::addUser(User& user, std::string password) 
{
    if (members.find(user.getSocketFd()) != members.end()) {
            user.sendErrorMessage(ERR_USERONCHANNEL, user,  user.getUsername() + " " + name + " :Is already on channel");
            throw std::invalid_argument("User already in the channel");
    }
    if (user_limit && static_cast<std::size_t>(user_limit) == members.size()){
            user.sendErrorMessage(ERR_CHANNELISFULL, user,  user.getUsername() + " " + name + " :Cannot join channel (+l)");
            throw std::runtime_error("Channel is full");
    }
    if (is_protected) {
        if (password.empty()) {
            user.sendErrorMessage(ERR_BADCHANNELKEY, user, name + " :Cannot join channel (+k)");
            throw std::invalid_argument("User did not provide password");
        } 
        if (password != this->password) {
            user.sendErrorMessage(ERR_BADCHANNELKEY, user, name + " :Cannot join channel (+k)");
            throw std::invalid_argument("Channel password incorrect");
        }
    }
    if (invite_only) {
        if (invited.find(user.getSocketFd()) == invited.end()) {
            user.sendErrorMessage(ERR_INVITEONLYCHAN, user, name + " :Cannot join channel (+i)");
            throw std::invalid_argument("User tried to join channel without invitation");
        }
        members.insert(std::make_pair(user.getSocketFd(), user));
        user.getChannels().push_back(name);
        invited.erase(user.getSocketFd());
        broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " JOIN " + name);
        return;
    }
    members.insert(std::make_pair(user.getSocketFd(), user));
    user.getChannels().push_back(name);
    if (members.size() == 1) {
        operators.insert(std::make_pair(user.getSocketFd(), user));
        logger.log(INFO, user.getNickname() + " is the operator of the channel " + name);
    }
    broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " JOIN " + name);
    if (!topic.empty())
        user.sendErrorMessage(RPL_TOPIC, user, name + " :" + topic);
    else
        user.sendErrorMessage(RPL_NOTOPIC, user, name + " :No topic is set");
    user.sendErrorMessage(RPL_NAMREPLY, user, "=" + name + " :" + listUsers());
    user.sendErrorMessage(RPL_ENDOFNAMES, user, name + " :End of /NAMES list");
    logger.log(DEBUG, "User joined channel: " + name + ", sending NAMES list: " + listUsers());
}

std::string Channel::listUsers()
{
    std::string list;
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it)
        list += it->second.getNickname()+ " ";
    if (!list.empty())
        list.erase(list.size() - 1);
    return list;
}

void Channel::partUser(User& user, std::string reason = "") 
{
    if (members.find(user.getSocketFd()) == members.end()) {
        user.sendErrorMessage(ERR_NOTONCHANNEL, user, name + " :You're not on that channel");
        throw std::invalid_argument("The user is not part of the channel " + name);
    }
    broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " PART " + name + " :" + reason);
    members.erase(user.getSocketFd());
    for(std::vector<std::string>::iterator it = user.getChannels().begin(); it != user.getChannels().end(); ++it) {
        if (*it == name) {
            it = user.getChannels().erase(it);
            break;
        }
    }
    operators.erase(user.getSocketFd());
    logger.log(INFO, "User " + user.getNickname() + " left channel " + name + (reason.empty() ? "" : " (Reason: " + reason + ")"));
}

void Channel::setTopic(User& user, const std::string& topic) 
{
    if (topic.empty()) {
        if (this->topic.empty())
            user.sendErrorMessage(RPL_NOTOPIC, user, name + " :No topic is set");
        else
            user.sendErrorMessage(RPL_TOPIC, user, name + " :" + this->topic);
        return;
    }
    if (!is_member(user)) {
        user.sendErrorMessage(ERR_NOTONCHANNEL, user, name + " :You're not in that channel");
        throw std::invalid_argument("The user is not part of the channel");
    }
    if (topic_restricted && !is_operator(user)) {
        user.sendErrorMessage(ERR_CHANOPRIVSNEEDED, user, name + " :You're not a channel operator");
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

std::map<int, User>& Channel::getMembers()
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
        user.sendErrorMessage(ERR_NOTONCHANNEL, user, name + " :You're not in that channel");
        throw std::invalid_argument("The user is not part of the channel");
    }

    if (!is_operator(user)) {
        user.sendErrorMessage(ERR_CHANOPRIVSNEEDED, user, name + " :You're not a channel operator");
        throw std::invalid_argument("The user is not a channel operator");
    }

    std::map<int, User>::iterator it;
    for (it = members.begin(); it != members.end(); ++it) {
        if (it->second.getNickname() == target)
            break;
    }
    if (it == members.end()) {
        user.sendErrorMessage(ERR_USERNOTINCHANNEL, user, target + " " + name + " :They aren't in that channel");
        throw std::invalid_argument("The target user is not in the channel");
    }
    User& target_user = it->second;
    broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " KICK " + name + " " + target + " :" + reason);
    target_user.sendMessage(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " KICK " + name + " " + target + " :" + reason, false);
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
        user.sendErrorMessage(ERR_NOTONCHANNEL, user, name + " :You're not in that channel");
        throw std::invalid_argument("The user is not part of the channel");
    }
    if (!is_operator(user)) {
        user.sendErrorMessage(ERR_CHANOPRIVSNEEDED, user, name + " :You're not a channel operator");
        throw std::invalid_argument("The user is not a channel operator");
    }
    std::map<int, User>::iterator it;
    for (it = Users.begin(); it != Users.end(); ++it) {
        if (it->second.getNickname() == target)
            break;
    }
    if (it == Users.end()) {
        user.sendErrorMessage(ERR_NOSUCHNICK, user, name + " :No such nick/channel");
        throw std::invalid_argument("The invited user does not exist");
    }
    User& target_user = it->second;
    if (is_member(target_user)) {
        user.sendErrorMessage(ERR_USERONCHANNEL, user, (target_user.getNickname().empty() ? "*" : target_user.getNickname()) + " " + name + " :is already in channel");
        throw std::invalid_argument("The invited user is already in the channel");
    }
    invited.insert(std::make_pair(target_user.getSocketFd(), target_user));
    user.sendErrorMessage(RPL_INVITING, user, target + " " + name);
    target_user.sendMessage(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " INVITE " + target_user.getNickname() + " :" + name, false);
}

void Channel::printMode(User& user) {
    if (!is_member(user)) {
        user.sendErrorMessage(ERR_NOTONCHANNEL, user, name + " :You're not in that channel");
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
    user.sendErrorMessage(RPL_CHANNELMODEIS, user, name + " " + modes + " " + params);
}

void Channel::changeMode(User& user, std::vector<std::string> params) 
{
    if (!is_member(user)) {
        user.sendErrorMessage(ERR_NOTONCHANNEL, user, name + " :You're not in that channel");
        throw std::invalid_argument("The user is not part of the channel");
    }
    if (params[0].length() == 1 && params[0][0] == 'b'){
        user.sendErrorMessage(RPL_ENDOFBANLIST, user, name + " :End of Channel Ban List");
        return;
    }
    if (!is_operator(user)) {
        user.sendErrorMessage(ERR_CHANOPRIVSNEEDED, user, name + " :You're not a channel operator");
        throw std::invalid_argument("The user is not a channel operator");
    }
    if (params[0].length() < 2 || (params[0][0] != '+' && params[0][0] != '-')) {
        user.sendErrorMessage(ERR_UNKNOWNMODE, user, name + " :Is unknown mode char");
        throw std::invalid_argument("Channel mode flag(s) incorrect");
    }
    bool enable = params[0][0] == '+';
    size_t param_index = 1;
    for (size_t i = 1; i < params[0].size(); i++) {
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
                if (enable) {
                    if (param_index >= params.size()) {
                        user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, " MODE :Not enough parameters");
                        throw std::invalid_argument("Missing the new channel password");
                    }
                    if (is_protected && !password.empty()) {
                        user.sendErrorMessage(ERR_KEYSET, user, " :Channel key already set");
                        throw std::invalid_argument("Channel password already set");
                    }
                    password = params[param_index++];
                }
                else
                    password.clear();
                is_protected = enable;
                break;
            case 'l':
            if (enable) {
                if (param_index >= params.size()) {
                    user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, " MODE :Not enough parameters");
                    throw std::invalid_argument("Missing the user limit parameter");
                }
                int limit = Utils::stringToInt(params[param_index++]);
                if (limit <= 0) {
                throw std::invalid_argument("User limit must be greater than zero");
                }
                user_limit = limit; 
                break;
            } 
            else {
                user_limit = 0; 
            }
            break;
            case 'o': {
                if (param_index >= params.size()) {
                    user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, " MODE :Not enough parameters");
                    throw std::invalid_argument("Missing the channel operator parameter");
                }
                std::map<int, User>::iterator it = members.end();
                for (std::map<int, User>::iterator it_tmp = members.begin(); it_tmp != members.end(); it_tmp++) {
                    if (params[param_index] == it_tmp->second.getNickname())
                        it = it_tmp;
                }
                if (it == members.end()){
                    user.sendErrorMessage(ERR_USERNOTINCHANNEL, user, params[param_index] + " " + name + " :They aren't in that channel");
                    throw std::invalid_argument("The target user is not in the channel");
                }
                if (enable){
                    if (is_operator(it->second)){
                        logger.log(WARNING, "User " + params[param_index] + " is already an operator of channel " +  name);
                        return;
                    }
                    operators.insert(std::make_pair(it->second.getSocketFd(), it->second));
                }
                else
                    operators.erase(operators.find(it->second.getSocketFd()));
                param_index++;
                break;
            }
            default:
                user.sendErrorMessage(ERR_UNKNOWNMODE, user, name + " :Is unknown mode char");
                throw std::invalid_argument("Channel mode flag(s) incorrect");
        }
    }
    broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " MODE " + name + " " + params[0]);
}

void Channel::broadcast(std::string msg, int excludedFd)
{
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it) {
        if (it->first == excludedFd)
            continue;
        it->second.sendMessage(msg, false);
    }
    logger.log(INFO, msg);
}