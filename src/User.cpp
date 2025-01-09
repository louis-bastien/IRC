#include "User.hpp"
#include "Errors.hpp"

User::User(int socket_fd, Logger& logger) : socket_fd(socket_fd), is_authenticated(false), is_registered(false), is_visible(true), logger(logger) {}

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
        realname = other.realname;
        socket_fd = other.socket_fd;
        is_authenticated = other.is_authenticated;
        channels = other.channels;
    }
    return (*this);
}

void User::setNickname(const std::string& nickname)
{
    if (this->is_registered) {
        sendErrorMessage(ERR_ALREADYREGISTERED, *this, " :You may not reregister");
        throw std::invalid_argument("User is already registered can't change nickname.");
    }
    if (nickname.empty()) {
        sendErrorMessage(ERR_NONICKNAMEGIVEN, *this, " :No nickname given");
        throw std::invalid_argument("Attempted to set an empty nickname.");
    }
    if (nickname.length() > 9) {
        sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, " :Erroneous nickname (too long)");
        throw std::invalid_argument("Nickname too long: " + nickname);
    }
    if (!(isalpha(nickname[0]) || nickname[0] == '-' || nickname[0] == '[' || nickname[0] == ']' ||
        nickname[0] == '\\' || nickname[0] == '^' || nickname[0] == '_')) {
        sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, nickname + " :Erroneous nickname");
        throw std::invalid_argument("Invalid nickname first character: " + nickname);
    }
    for (std::string::size_type i = 0; i < nickname.length(); ++i) 
    {
        char c = nickname[i];
        if (!(isalnum(c) || c == '-' || c == '[' || c == ']' ||
        c == '\\' || c == '^' || c == '_')) {
            sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, nickname + " :Erroneous nickname");
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

std::vector<std::string>& User::getChannels()
{
    return (this->channels);
}

void User::setUsername(const std::string& username)
{
    if (this->is_registered) {
        sendErrorMessage(ERR_ALREADYREGISTERED, *this, " :You may not reregister");
        throw std::invalid_argument("User is already registered can't change username.");
    }
    if (!this->is_authenticated) {
        sendErrorMessage(ERR_NOTREGISTERED, *this, " :You are not authenticated");
        throw std::invalid_argument("User is not yet authenticated");
    }
    if (username.empty()) {
        sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, username + " :Erroneous username");
        throw std::invalid_argument("Attempted to set an empty username");
    }
    if (!isalpha(username[0]) && username[0] != '-'){
        sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, username + " :Erroneous username");
        throw std::invalid_argument("Attempted to set invalid username");
    }
    for (std::string::size_type i = 0; i < username.length(); ++i)
    {
        char c = username[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') {
            sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, username + " :Erroneous username");
            throw std::invalid_argument("Attempted to set invalid username");
        }
    }
    this->username = username;
    logger.log(INFO, "Username set to " + username);
}

std::string User::getHostname() const
{
    return (this->hostname);
}

std::string User::getRealname() const
{
    return (this->realname);
}

void User::setHostname(const std::string& hostname)
{
    if (this->is_registered) {
        sendErrorMessage(ERR_ALREADYREGISTERED, *this, " :You may not reregister");
        throw std::invalid_argument("User is already registered can't change hostname");
    }
    if (!this->is_authenticated) {
        sendErrorMessage(ERR_NOTREGISTERED, *this, " :You are not authenticated");
        throw std::invalid_argument("User is not yet authenticated");
    }
    if (hostname.empty()) {
        sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, hostname + " :Erroneous hostname");
        throw std::invalid_argument("Attempted to set an empty hostname");
    }
    if (!isalpha(hostname[0]) && hostname[0] != '-') {
        sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, hostname + " :Erroneous hostname");
        throw std::invalid_argument("Attempted to set invalid hostname");
    }
    for (std::string::size_type i = 0; i < hostname.length(); ++i)
    {
        char c = hostname[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') {
            sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, hostname + " :Erroneous hostname");
            throw std::invalid_argument("Attempted to set invalid hostname");
        }
    }
    this->hostname = hostname;
    logger.log(INFO, "Hostname set to " + hostname);
}

void User::setRealname(const std::string& realname)
{
    if (this->is_registered) {
        sendErrorMessage(ERR_ALREADYREGISTERED, *this, " :You may not reregister");
        throw std::invalid_argument("User is already registered can't change realname");
    }
    if (!this->is_authenticated) {
        sendErrorMessage(ERR_NOTREGISTERED, *this, " :You are not authenticated");
        throw std::invalid_argument("User is not yet authenticated");
    }
    if (realname.empty()) {
        sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, realname + " :Erroneous realname");
        throw std::invalid_argument("Attempted to set an empty realname");
    }
    if (!isalpha(realname[0]) && realname[0] != '-') {
        sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, realname + " :Erroneous realname");
        throw std::invalid_argument("Attempted to set invalid realname");
    }
    for (std::string::size_type i = 0; i < realname.length(); ++i)
    {
        char c = realname[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') {
            sendErrorMessage(ERR_ERRONEUSNICKNAME, *this, realname + " :Erroneous realname");
            throw std::invalid_argument("Attempted to set invalid realname");
        }
    }
    this->realname = realname;
    logger.log(INFO, "realname set to " + realname);
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
        sendErrorMessage(ERR_ALREADYREGISTERED, *this, " :You are already registered");
        throw std::invalid_argument("User is already registered");
    }
    this->is_authenticated = true;
    logger.log(INFO, "User authenticated successfully");
}


void User::doRegister(Server& server)
{
    if (!is_authenticated || username.empty() || nickname.empty() || hostname.empty())
        return;
    this->is_registered = true;
    sendErrorMessage(RPL_WELCOME, *this, ":Welcome to our IRC server");
    sendErrorMessage(RPL_YOURHOST, *this, ":Your host is running version 1.0");
    sendErrorMessage(RPL_CREATED, *this, ":This server was created by Anna and Louis");
    sendErrorMessage(RPL_LUSERCHANNELS, *this, ":We are compatible and compliant with basic IRC commands");
    sendErrorMessage(RPL_ISUPPORT, *this, ":CHANTYPES=# PREFIX=(ov)@+");
    sendINFO(server);
    sendMOTD(server);
    logger.log(INFO, "User " + nickname + " registered successfully.");
}

void User::sendMOTD(Server& server) {
    if (server.getMOTD().empty())
        sendErrorMessage(ERR_NOMOTD, *this, ":MOTD File is missing");
    else {
        sendErrorMessage(RPL_MOTD, *this, ":" + server.getMOTD());
        sendErrorMessage(RPL_ENDOFMOTD, *this, ":End of MOTD command");
    }
}

void User::sendINFO(Server& server) {

    std::string userNumbers = Utils::toString(server.getUserMap().size());
    std::string channelNumbers =Utils::toString(server.getChannelMap().size());

    sendErrorMessage(RPL_LUSERCLIENT, *this, ":There are " + userNumbers + " clients on this server");
    sendErrorMessage(RPL_LUSEROP, *this, ":Number of operators online unknown");
    sendErrorMessage(RPL_LUSERUNKNOWN, *this, ":Number of unknown connections unknown");
    sendErrorMessage(ERR_NOMOTD, *this, channelNumbers + " :Channels formed");
}

void User::sendMessage(const std::string message, bool serverPrefix)
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

void User::sendErrorMessage(int errorCode, User& user, std::string message)
{
    std::string formattedMessage;
    std::string paddedErrorCode = Utils::padLeft(errorCode, '0', 3);

    std::string userNickname = user.getNickname().empty() ? "*" : user.getNickname();
    formattedMessage = ":ircserv " + paddedErrorCode + " " + userNickname + " " + message + "\r\n";

    if (send(user.getSocketFd(), formattedMessage.c_str(), formattedMessage.length(), 0) == -1) 
        logger.log(ERROR, "Failed to send message: " + message);
    logger.log(DEBUG, "Sent message (fd=" + Utils::toString(socket_fd) + "): '" + formattedMessage);
}

void User::changeMode(std::vector<std::string> params) 
{
    if (params[0].length() < 2 || (params[0][0] != '+' && params[0][0] != '-')) {
        sendErrorMessage(ERR_UNKNOWNMODE, *this, ":Is unknown mode char");
        throw std::invalid_argument("User mode flag(s) incorrect");
    }
    bool enable = params[0][0] == '+';
    for (size_t i = 1; i < params[0].size(); i++) {
        char mode = params[0][i];
        switch (mode) 
        {
            case 'i':
                is_visible = enable;
                break;
            default:
                sendErrorMessage(ERR_UNKNOWNMODE, *this, " :Is unknown mode char");
                throw std::invalid_argument("User mode flag(s) incorrect");
        }
    }
    sendErrorMessage(RPL_UMODEIS, *this, params[0]);
}


void User::leaveAllChannels(std::map<std::string, Channel>& allChannels) 
{
    std::vector<std::string> channelsCopy = channels;
    for (std::vector<std::string>::iterator it = channelsCopy.begin(); it != channelsCopy.end(); ++it)
    {
        std::string channelName = *it;
        std::map<std::string, Channel>::iterator channelIt = allChannels.find(channelName);
        if (channelIt != allChannels.end()) 
            channelIt->second.partUser(*this, "Leaving all channels");
    }
    channels.clear();
    logger.log(INFO, nickname + " left all channels.");
}

int User::getSocketFd()
{
    return(this->socket_fd);
}
