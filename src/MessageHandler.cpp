#include "MessageHandler.hpp"

std::map<std::string, MessageHandler::CommandHandler> MessageHandler::_cmdHandlers;

void MessageHandler::_initCmdHandlers() {
    if (_cmdHandlers.empty()) {
        MessageHandler::_cmdHandlers.insert(std::make_pair("CAP", &MessageHandler::_handleCAP));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PASS", &MessageHandler::_handlePASS));
        MessageHandler::_cmdHandlers.insert(std::make_pair("NICK", &MessageHandler::_handleNICK));
        MessageHandler::_cmdHandlers.insert(std::make_pair("USER", &MessageHandler::_handleUSER));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PING", &MessageHandler::_handlePING));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PING", &MessageHandler::_handleJOIN));
    }
}

void MessageHandler::validateAndDispatch(int clientFd, const Message& message, Server& server) {
    _initCmdHandlers();
    std::map<int, User>::iterator itUser = server.getUserMap().find(clientFd);
    if (itUser == server.getUserMap().end())
            throw::std::runtime_error("Could not find client");

    std::map<std::string, CommandHandler>::iterator itCmd = _cmdHandlers.find(message.getCommand());
    if (itCmd != _cmdHandlers.end())
        (itCmd->second)(itUser->second, message, server);
    else
        throw std::invalid_argument("Invalid Command: " + message.getCommand());
}

void MessageHandler::_handleCAP(User& user, const Message& message, Server& server) {
    if (!_validateCAP(message))
        throw std::invalid_argument("Wrong command format for CAP");
    if (message.getParams()[0] == "LS")
        user.sendMessage("CAP * LS :");
    else if (message.getParams()[0] == "END")
        server.getLogger().log(DEBUG, "CAP END aknowledged");
    server.getLogger().log(DEBUG, "CAP command handled");
}

void MessageHandler::_handlePASS(User& user, const Message& message, Server& server) {    
    if (!_validatePASS(message))
        throw std::invalid_argument("Wrong command format for PASS");
    if (message.getParams()[0] == server.getPassword())
                    user.authenticate();
    else {
        user.sendMessage("464 ERR_PASSWDMISMATCH");
        server.getLogger().log(WARNING, "Wrong Password Received");
    }
    server.getLogger().log(DEBUG, "PASS command handled");
}

void MessageHandler::_handleNICK(User& user, const Message& message, Server& server) {
    if (!_isAuthenticated(user, message.getCommand(), server)) return;
    user.setNickname(message.getParams()[0]);
    server.getLogger().log(DEBUG, "NICK command handled");
}

void MessageHandler::_handleUSER(User& user, const Message& message, Server& server) {
    if (!_isAuthenticated(user, message.getCommand(), server)) return;
    user.setUsername(message.getParams()[0]);
    server.getLogger().log(DEBUG, "USER command handled");
}


void MessageHandler::_handlePING(User& user, const Message& message, Server& server) {
    if (message.getParams().empty())
        user.sendMessage("409 ERR_NOORIGIN: No origin specified");
    else
        user.sendMessage("PONG " + message.getParams()[0]);
    server.getLogger().log(DEBUG, "PING command handled");
}

void MessageHandler::_handleJOIN(User& user, const Message& message, Server& server) {
    if (!_validateJOIN(message))
        throw std::invalid_argument("Wrong command format for JOIN");  
    if (!_isRegistered(user, message.getCommand(), server)) return;

    std::vector<std::string> channelNames = Utils::split(message.getParams()[0], ',');
    std::vector<std::string> keys = Utils::split(message.getParams()[1], ',');
    std::map<std::string, Channel>& channelMap = server.getChannelMap();

    while(!channelNames.empty()) {
        std::string currentChannel = channelNames.front();
        std::map<std::string, Channel>::iterator it = channelMap.find(channelNames.front());
        if (it == channelMap.end())
            it = channelMap.insert(std::make_pair(channelNames.front(), Channel(channelNames.front(), server.getLogger()))).first;
        if (it->second.isProtected()) {
            if (!keys.empty() && !keys.front().empty()) {
                try { 
                    it->second.addUser(user, keys.front());
                    keys.erase(keys.begin());
                } catch (std::exception& e) {
                    server.getLogger().log(WARNING, e.what());
                }
            }
            else
                throw std::invalid_argument("Channel requires a valid password");
        }
        else 
            it->second.addUser(user);
        channelNames.erase(channelNames.begin());
    }
}




bool MessageHandler::_isAuthenticated(User& user, const std::string& command, Server& server) {
    if (!user.isAuthenticated()) {
        user.sendMessage("464 ERR_PASSWDMISMATCH");
        server.getLogger().log(WARNING, "Unauthorized command: " + command);
        return false;
    }
    return true;
}

bool MessageHandler::_isRegistered(User& user, const std::string& command, Server& server) {
    if (!user.isRegistered()) {
        user.sendMessage("451 ERR_NOTREGISTERED");
        server.getLogger().log(WARNING, "Unauthorized command: " + command);
        return false;
    }
    return true;
}

bool MessageHandler::_validateCAP(const Message& message) {
    return message.getTrailing().empty() && ((message.getParams()[0] == "LS" || message.getParams()[0] == "END"));
}

bool MessageHandler::_validatePASS(const Message& message) {
    return message.getParams().size() == 1 && message.getTrailing().empty();
}

bool MessageHandler::_validateNICK(const Message& message) {
    return message.getParams().size() == 1 && message.getTrailing().empty();
}

bool MessageHandler::_validateUSER(const Message& message) {
    return message.getParams().size() == 3 && !message.getTrailing().empty();
}

bool MessageHandler::_validateJOIN(const Message& message) {
    return (message.getParams().size() == 1 || message.getParams().size() == 2) && !message.getTrailing().empty();
}

/*
Required Commands
These commands are necessary for the core functionality specified in your subject:

Authentication

PASS <password>
Used to authenticate a user with a password.
If your server requires a password, implement this command to validate the user's connection.

NICK <nickname>
Sets or changes the user's nickname.
Required to identify users uniquely in the IRC server.

USER <username> <hostname> <servername> :<realname>
Sends user information to the server during connection.
Fields like <hostname> and <servername> can be placeholders since you are not building a distributed IRC server.

Channel Management

JOIN <channel>{,<channel>} [<key>{,<key>}]
Allows users to join channels.
Handle invite-only channels (MODE +i) and channels with keys (MODE +k).

PART <channel>{,<channel>} [<message>]
Allows users to leave channels.

KICK <channel> <user> [<comment>]
Allows a channel operator to remove a user from the channel.

INVITE <nickname> <channel>
Allows a channel operator to invite a user to a channel.

TOPIC <channel> [<topic>]
Allows users to view the topic of a channel.
Only channel operators can set the topic if the channel is in +t mode.

MODE <channel> <modes> [<params>]
Allows channel operators to set modes:
+i: Invite-only mode.
+t: Topic changes restricted to operators.
+k <key>: Set channel password.
+o <nickname>: Grant operator status.
+l <limit>: Set maximum user limit.

Messaging

PRIVMSG <receiver>{,<receiver>} :<message>
Sends a private message to a user or channel.

NOTICE <receiver>{,<receiver>} :<message>
Sends a notice to a user or channel (non-replyable).
*/