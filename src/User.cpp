#include "User.hpp"
// CREATE A FUNCTION LEAVE ALL CHANNELS!!!!


User::User(int socket_fd) : username(""), nickname("*"), socket_fd(socket_fd),
            is_authenticated(false), is_operator(false) {}

std::string User::getNickname() const
{
    return (this->nickname);
}

User::~User(){}

// here i only check certain parameters, but i DONT check if the nickname already exists, 
//it should be done in server or a channel
// i think i need to pront messages here like "... set their name as ..." , but not sure if to do it here

void User::setNickname(const std::string& nickname)
{
    if (nickname.empty()) 
        sendMessage("431 ERR_NONICKNAMEGIVEN: No nickname provided.");
    if (nickname.length() > 9)
        sendMessage("432 ERR_ERRONEUSNICKNAME: Nickname is too long.");
    if (!(isalpha(nickname[0]) || nickname[0] == '-' || nickname[0] == '[' || nickname[0] == ']' ||
        nickname[0] == '\\' || nickname[0] == '^' || nickname[0] == '_'))
        sendMessage("432 ERR_ERRONEUSNICKNAME: Invalid first character.");
    for (std::string::size_type i = 0; i < nickname.length(); ++i) 
    {
        char c = nickname[i];
        if (!(isalnum(c) || c == '-' || c == '[' || c == ']' ||
        c == '\\' || c == '^' || c == '_'))
            sendMessage("432 ERR_ERRONEUSNICKNAME: Invalid character in nickname.");
    }
    this->nickname = nickname;
}

std::string User::getUsername() const
{
    return (this->username);
}


//same as in nickname, not all params are checked and need to pront msgs either here or somewhere
void User::setUsername(const std::string& username)
{
    if (username.empty())
        sendMessage("432 ERR_ERRONEUSNICKNAME: Invalid username.");
    if (!isalpha(username[0]) && username[0] != '-')
        sendMessage("432 ERR_ERRONEUSNICKNAME: Invalid username.");
    for (std::string::size_type i = 0; i < nickname.length(); ++i)
    {
        char c = nickname[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '.') 
        sendMessage("432 ERR_ERRONEUSNICKNAME: Invalid username.");
    }
    this->username = username;
}

bool User::isAuthenticated() const
{
    return(is_authenticated);
}

// i think this has to also be done outside user to verify the password, or here with 2 params, expected pass and given by the user pass where we compare them

void User::authenticate()
{
    if (is_authenticated) 
        sendMessage("433 ERR_ALREADYREGISTERED: Already authenticated.");
    this->is_authenticated = true;
}


//also i am not sure about sending private messages, i also think it shoulb be done outside the usres class to access the target
void User::sendMessage(const std::string& message)
{
    std::string formattedMessage = ":ircserv " + message + "\r\n";
    
    if (send(socket_fd, formattedMessage.c_str(), formattedMessage.length(), 0) == -1) 
        std::cerr << "Failed to send message to user: " << std::endl;
}





