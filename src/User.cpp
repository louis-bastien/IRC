#include "User.hpp"

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
        sendMessage("462 ERR_ALREADYREGISTRED: User already registered.");
        throw std::invalid_argument("User is already registered can't change nickname.");
    }
    if (nickname.empty()) {
        sendMessage("431 ERR_NONICKNAMEGIVEN: No nickname provided.");
        throw std::invalid_argument("Attempted to set an empty nickname.");
    }
    if (nickname.length() > 9) {
        sendMessage("432 ERR_ERRONEUSNICKNAME: Nickname is too long.");
        throw std::invalid_argument("Nickname too long: " + nickname);
    }
    if (!(isalpha(nickname[0]) || nickname[0] == '-' || nickname[0] == '[' || nickname[0] == ']' ||
        nickname[0] == '\\' || nickname[0] == '^' || nickname[0] == '_')) {
        sendMessage("432 ERR_ERRONEUSNICKNAME: Invalid first character in nickname.");
        throw std::invalid_argument("Invalid nickname first character: " + nickname);
    }
    for (std::string::size_type i = 0; i < nickname.length(); ++i) 
    {
        char c = nickname[i];
        if (!(isalnum(c) || c == '-' || c == '[' || c == ']' ||
        c == '\\' || c == '^' || c == '_')) {
            sendMessage("432 ERR_ERRONEUSNICKNAME: Invalid character in nickname.");
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
        sendMessage("462 ERR_ALREADYREGISTRED: User already registered.");
        throw std::invalid_argument("User is already registered can't change username.");
    }
    if (username.empty()) {
        sendMessage("432 ERR_ERRONEUSUSERNAME: Invalid username.");
        throw std::invalid_argument("Attempted to set an empty username.");
    }
    if (!isalpha(username[0]) && username[0] != '-'){
        sendMessage("432 ERR_ERRONEUSUSERNAME: Invalid username.");
        throw std::invalid_argument("Attempted to set invalid username.");
    }
    for (std::string::size_type i = 0; i < username.length(); ++i)
    {
        char c = username[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') {
            sendMessage("432 ERR_ERRONEUSUSERNAME: Invalid username.");
            throw std::invalid_argument("Attempted to set an empty username.");
        }
    }
    this->username = username;
    logger.log(INFO, "Username set to " + username);
    if (canRegister())
        doRegister();
}

bool User::isAuthenticated() const
{
    return(is_authenticated);
}

bool User::isRegistered() const
{
    return(is_registered);
}

bool User::canRegister() const
{
    return is_authenticated && !nickname.empty() && !username.empty();
}

void User::authenticate()
{
    if (is_authenticated) {
        sendMessage("433 ERR_ALREADYREGISTERED: Already authenticated.");
        logger.log(WARNING, "User attempted re-authentication. Ignoring.");
        return;
    }
    this->is_authenticated = true;
    logger.log(INFO, "User authenticated successfully.");
}

void User::doRegister()
{
    this->is_registered = true;
    logger.log(INFO, "User registered successfully.");

    // Send registration completion messages
    sendMessage("001 " + nickname + " :Welcome to the IRC server");
    sendMessage("002 " + nickname + " :Your host is ircserv, running beta version 0.2");
    sendMessage("003 " + nickname + " :This server was created for 42 School Project by adanylev and lbastien");
    sendMessage("004 " + nickname + " Not many functions available yet");
}

void User::sendMessage(const std::string& message)
{
    std::string formattedMessage = ":ircserv " + message + "\r\n";
    if (send(socket_fd, formattedMessage.c_str(), formattedMessage.length(), 0) == -1) 
        logger.log(ERROR, "Failed to send message: " + message);
    logger.log(DEBUG, "Sent message '" + message + "' to client fd=" + Utils::toString(socket_fd));    
}

void User::leaveChannel(Channel& channel, std::string& reason) 
{
    channel.removeUser(*this, reason);
    channels.erase(std::remove(channels.begin(), channels.end(), channel.getName()), channels.end());
    logger.log(INFO, nickname + " left channel " + channel.getName());
}

void User::leaveAllChannels(std::map<std::string, Channel>& allChannels) 
{
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it)
    {
        std::string channelName = *it;
        std::map<std::string, Channel>::iterator channelIt = allChannels.find(channelName);
        if (channelIt != allChannels.end()) 
            channelIt->second.removeUser(*this, "Goodbye");
    }
    channels.clear();
    logger.log(INFO, nickname + " left all channels.");
}

int User::getSocketFd()
{
    return(this->socket_fd);
}
