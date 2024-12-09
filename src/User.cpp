#include "User.hpp"

User::User(int socket_fd, Logger& logger) : username(""), nickname("*"), socket_fd(socket_fd),
            is_authenticated(false),is_registered(false), logger(logger) {}

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

// here i only check certain parameters, but i DONT check if the nickname already exists, 
//it should be done in server or a channel
// i think i need to prompt messages here like "... set their name as ..." , but not sure if to do it here

void User::setNickname(const std::string& nickname)
{
   if (nickname.empty()) {
        sendMessage("431 ERR_NONICKNAMEGIVEN: No nickname provided.");
        logger.log(WARNING, "Attempted to set an empty nickname.");
        return;
    }
    if (nickname.length() > 9) {
        sendMessage("432 ERR_ERRONEUSNICKNAME: Nickname is too long.");
        logger.log(WARNING, "Nickname too long: " + nickname);
        return;
    }
    if (!(isalpha(nickname[0]) || nickname[0] == '-' || nickname[0] == '[' || nickname[0] == ']' ||
        nickname[0] == '\\' || nickname[0] == '^' || nickname[0] == '_')) {
        sendMessage("432 ERR_ERRONEUSNICKNAME: Invalid first character in nickname.");
        logger.log(WARNING, "Invalid nickname first character: " + nickname);
        return;
    }
    for (std::string::size_type i = 0; i < nickname.length(); ++i) 
    {
        char c = nickname[i];
        if (!(isalnum(c) || c == '-' || c == '[' || c == ']' ||
        c == '\\' || c == '^' || c == '_')) {
            sendMessage("432 ERR_ERRONEUSNICKNAME: Invalid character in nickname.");
            logger.log(WARNING, "Invalid character in nickname: " + nickname);
            return;
        }
    }
    this->nickname = nickname;
    logger.log(INFO, "Nickname set to " + nickname);
}

std::string User::getUsername() const
{
    return (this->username);
}


//same as in nickname, not all params are checked and need to promt msgs either here or somewhere
void User::setUsername(const std::string& username)
{
    if (username.empty()) {
        sendMessage("432 ERR_ERRONEUSUSERNAME: Invalid username.");
        logger.log(WARNING, "Attempted to set an empty username.");
        return;
    }
    if (!isalpha(username[0]) && username[0] != '-'){
        sendMessage("432 ERR_ERRONEUSUSERNAME: Invalid username.");
        logger.log(WARNING, "Attempted to set invalid username.");
        return;
    }
    for (std::string::size_type i = 0; i < nickname.length(); ++i)
    {
        char c = nickname[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') {
            sendMessage("432 ERR_ERRONEUSUSERNAME: Invalid username.");
            logger.log(WARNING, "Attempted to set an empty username.");
            return;
        }
    }
    this->username = username;
    logger.log(INFO, "Username set to " + username);
}

bool User::isAuthenticated() const
{
    return(is_authenticated);
}

bool User::isRegistered() const
{
    return(is_registered);
}

// i think this has to also be done outside user to verify the password, or here with 2 params, expected pass and given by the user pass where we compare them

void User::authenticate()
{
    if (is_authenticated) {
        sendMessage("433 ERR_ALREADYREGISTERED: Already authenticated.");
        logger.log(WARNING, "User attempted re-authentication.");
        return;
    }
    this->is_authenticated = true;
    logger.log(INFO, "User authenticated successfully.");
}


//also i am not sure about sending private messages, i also think it shoulb be done outside the usres class to access the target
void User::sendMessage(const std::string& message)
{
    std::string formattedMessage = ":ircserv " + message + "\r\n";
    if (send(socket_fd, formattedMessage.c_str(), formattedMessage.length(), 0) == -1) 
        logger.log(ERROR, "Failed to send message: " + message);
    logger.log(DEBUG, "Sent message '" + message + "' to client fd=" + Utils::toString(socket_fd));    
}


void User::joinChannel(Channel& channel) 
{
    channel.addUser(*this);
    channels.push_back(channel.getName());
    logger.log(INFO, nickname + " joined channel " + channel.getName());
}

void User::leaveChannel(Channel& channel) 
{
    channel.removeUser(*this);
    channels.erase(std::remove(channels.begin(), channels.end(), channel.getName()), channels.end());
    logger.log(INFO, nickname + " left channel " + channel.getName());
}

void User::leaveAllChannels(std::map<std::string, Channel>& allChannels) 
{
    for (std::vector<std::string>::iterator it = channels.begin(); it != channels.end(); ++it) 
    {
        std::map<std::string, Channel>::iterator channelIt = allChannels.find(*it);
        if (channelIt != allChannels.end()) 
            channelIt->second.removeUser(*this);
    }
    channels.clear();
    logger.log(INFO, nickname + " left all channels.");
}

int User::getSocketFd()
{
    return(this->socket_fd);
}