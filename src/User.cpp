#include "User.hpp"
#include "Errors.hpp"

User::User(int socket_fd, Logger& logger) : username(""), nickname("*"), socket_fd(socket_fd),
            is_authenticated(false), is_registered(false), logger(logger) {}

std::string User::getNickname() const
{
    return (this->nickname);
}

User::~User()
{}

User& User::operator=(const User& other) 
{
    if (this != &other)
    {
        username = other.username;
        nickname = other.nickname;
        socket_fd = other.socket_fd;
        is_authenticated = other.is_authenticated;
        channels = other.channels;
    }
    return (*this);
}

void User::setNickname(const std::string& nickname)
{
    if (this->is_registered) {
        sendMessage(ERR_ALREADYREGISTERED + " " + getNickname().empty() ? "*" : getNickname() + " :You may not reregister");
        throw std::invalid_argument("User is already registered can't change nickname.");
    }
    if (nickname.empty()) {
        sendMessage(ERR_NONICKNAMEGIVEN + " " + getNickname().empty() ? "*" : getNickname() + " :No nickname given");
        throw std::invalid_argument("Attempted to set an empty nickname.");
    }
    if (nickname.length() > 9) {
        sendMessage(ERR_ERRONEUSNICKNAME + " " + getNickname().empty() ? "*" : getNickname() + " :Erroneous nickname (too long)");
        throw std::invalid_argument("Nickname too long: " + nickname);
    }
    if (!(isalpha(nickname[0]) || nickname[0] == '-' || nickname[0] == '[' || nickname[0] == ']' ||
        nickname[0] == '\\' || nickname[0] == '^' || nickname[0] == '_')) {
        sendMessage(ERR_ERRONEUSNICKNAME +  " " + getNickname().empty() ? "*" : getNickname() + " " + nickname + " :Erroneous nickname");
        throw std::invalid_argument("Invalid nickname first character: " + nickname);
    }
    for (std::string::size_type i = 0; i < nickname.length(); ++i) 
    {
        char c = nickname[i];
        if (!(isalnum(c) || c == '-' || c == '[' || c == ']' ||
        c == '\\' || c == '^' || c == '_')) {
            sendMessage(ERR_ERRONEUSNICKNAME + " " + getNickname().empty() ? "*" : getNickname() + " " + nickname + " :Erroneous nickname");
            throw std::invalid_argument("Invalid character in nickname: " + nickname);
        }
    }
    this->nickname = nickname;
    logger.log(INFO, "Nickname set to " + nickname);
}

std::string User::getUsername() const
{
    return (this->username);
}

void User::setUsername(const std::string& username)
{
    if (this->is_registered) {
        sendMessage(ERR_ALREADYREGISTERED + " " + getNickname().empty() ? "*" : getNickname() + " :You may not reregister");
        throw std::invalid_argument("User is already registered can't change username.");
    }
    if (!this->is_authenticated) {
        sendMessage(ERR_NOTREGISTERED + " " + getNickname().empty() ? "*" : getNickname() + " :You are not authenticated");
        throw std::invalid_argument("User is not yet authenticated");
    }
    if (username.empty()) {
        sendMessage(ERR_ERRONEUSNICKNAME + " " + getNickname().empty() ? "*" : getNickname() + " " + username + " :Erroneous username");
        throw std::invalid_argument("Attempted to set an empty username.");
    }
    if (!isalpha(username[0]) && username[0] != '-'){
        sendMessage(ERR_ERRONEUSNICKNAME + " " + getNickname().empty() ? "*" : getNickname() + " " + username + " :Erroneous username");
        throw std::invalid_argument("Attempted to set invalid username.");
    }
    for (std::string::size_type i = 0; i < username.length(); ++i)
    {
        char c = username[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') {
            sendMessage(ERR_ERRONEUSNICKNAME + " " + getNickname().empty() ? "*" : getNickname() + " " + username + " :Erroneous username");
            throw std::invalid_argument("Attempted to set invalid username.");
        }
    }
    this->username = username;
    logger.log(INFO, "Username set to " + username);
}

std::string User::getHostname() const
{
    return (this->hostname);
}

void User::setHostname(const std::string& hostname)
{
    if (this->is_registered) {
        sendMessage(ERR_ALREADYREGISTERED + " " + getNickname().empty() ? "*" : getNickname() + " :You may not reregister");
        throw std::invalid_argument("User is already registered can't change hostname.");
    }
    if (!this->is_authenticated) {
        sendMessage(ERR_NOTREGISTERED + " " + getNickname().empty() ? "*" : getNickname() + " :You are not authenticated");
        throw std::invalid_argument("User is not yet authenticated");
    }
    if (hostname.empty()) {
        sendMessage(ERR_ERRONEUSNICKNAME + " " + getNickname().empty() ? "*" : getNickname() + " " + hostname + " :Erroneous hostname");
        throw std::invalid_argument("Attempted to set an empty hostname.");
    }
    if (!isalpha(hostname[0]) && hostname[0] != '-') {
        sendMessage(ERR_ERRONEUSNICKNAME + " " + getNickname().empty() ? "*" : getNickname() + " " + hostname + " :Erroneous hostname");
        throw std::invalid_argument("Attempted to set invalid hostname.");
    }
    for (std::string::size_type i = 0; i < hostname.length(); ++i)
    {
        char c = hostname[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') {
            sendMessage(ERR_ERRONEUSNICKNAME + " " + getNickname().empty() ? "*" : getNickname() + " " + hostname + " :Erroneous hostname");
            throw std::invalid_argument("Attempted to set invalid hostname.");
        }
    }
    this->hostname = hostname;
    logger.log(INFO, "Hostname set to " + hostname);
}

bool User::isAuthenticated() const
{
    return(is_authenticated);
}

bool User::isRegistered() const
{
    return(is_registered);
}

void User::authenticate()
{
    if (this->is_registered) {
        sendMessage(ERR_ALREADYREGISTERED + " " + getNickname().empty() ? "*" : getNickname() + " :You are already reigstered");
        throw std::invalid_argument("User is already registered.");
    }
    this->is_authenticated = true;
    logger.log(INFO, "User authenticated successfully.");
}

void User::doRegister()
{
    if (is_authenticated && !username.empty() && !nickname.empty() && !hostname.empty())
        this->is_registered = true;
    sendMessage(RPL_WELCOME + " " + nickname + " :Welcome to the our IRC server");
    sendMessage(RPL_YOURHOST + " " + nickname + " :Your host is running version 1.0");
    sendMessage(RPL_CREATED + " " + nickname + " :This server was created by Anna and Louis");
    sendMessage(RPL_MYINFO + " " + nickname + "We are compatible and compliant with basic IRC commands");
    logger.log(INFO, "User " + nickname + "registered successfully.");
}

void User::sendMessage(const std::string& message, bool serverPrefix)
{
    std::string formattedMessage;
    if (serverPrefix)
        formattedMessage = ":ircserv " + message + "\r\n";
    else
        formattedMessage = message + "\r\n";
    if (send(socket_fd, formattedMessage.c_str(), formattedMessage.length(), 0) == -1) 
        logger.log(ERROR, "Failed to send message: " + message);
    logger.log(DEBUG, "Sent message '" + message + "' to client fd: " + Utils::toString(socket_fd));    
}

void User::leaveAllChannels(std::map<std::string, Channel>& allChannels) 
{
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        std::string channelName = *it;
        std::map<std::string, Channel>::iterator channelIt = allChannels.find(channelName);
        if (channelIt != allChannels.end()) 
            channelIt->second.partUser(*this, "Leaving server");
    }
    channels.clear();
    logger.log(INFO, nickname + " left all channels.");
}

int User::getSocketFd()
{
    return(this->socket_fd);
}
