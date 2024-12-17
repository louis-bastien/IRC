#include "MessageHandler.hpp"
#include "Errors.hpp"

std::map<std::string, MessageHandler::CommandHandler> MessageHandler::_cmdHandlers;

void MessageHandler::_initCmdHandlers() {
    if (_cmdHandlers.empty()) {
        MessageHandler::_cmdHandlers.insert(std::make_pair("CAP", &MessageHandler::_handleCAP));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PASS", &MessageHandler::_handlePASS));
        MessageHandler::_cmdHandlers.insert(std::make_pair("NICK", &MessageHandler::_handleNICK));
        MessageHandler::_cmdHandlers.insert(std::make_pair("USER", &MessageHandler::_handleUSER));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PING", &MessageHandler::_handlePING));
        MessageHandler::_cmdHandlers.insert(std::make_pair("JOIN", &MessageHandler::_handleJOIN));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PART", &MessageHandler::_handlePART));
        MessageHandler::_cmdHandlers.insert(std::make_pair("KICK", &MessageHandler::_handleKICK));
        MessageHandler::_cmdHandlers.insert(std::make_pair("INVITE", &MessageHandler::_handleINVITE));
        MessageHandler::_cmdHandlers.insert(std::make_pair("TOPIC", &MessageHandler::_handleTOPIC));
        MessageHandler::_cmdHandlers.insert(std::make_pair("MODE", &MessageHandler::_handleMODE));
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
    validateCAP(message);
    if (message.getParams()[0] == "LS")
        user.sendMessage("CAP * LS :");
    else if (message.getParams()[0] == "END")
        server.getLogger().log(DEBUG, "CAP END aknowledged");
}

void MessageHandler::_handlePASS(User& user, const Message& message, Server& server) {    
    validatePASS(user, message);
    std::string password = message.getParams()[0];
    if (password != server.getPassword()) {
        user.sendMessage(ERR_PASSWDMISMATCH + " " + user.getNickname().empty() ? "*" : user.getNickname() + " :Password incorrect");
        throw std::invalid_argument("Incorrect password provided: " + password);
    }
    user.authenticate();
}

void MessageHandler::_handleNICK(User& user, const Message& message, Server& server) {
    validateNICK(user, message);
    std::string nickname = message.getParams()[0];
    std::map<int, User>::iterator it = server.getUserMap().begin();
    while (it != server.getUserMap().end()) {
        if (it->second.getNickname() == nickname) {
            user.sendMessage(ERR_NICKNAMEINUSE + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + nickname + " :Nickname is already in use");
            throw std::invalid_argument("Nickname already exists: " + nickname);
        }
        it++;
    }
    user.setNickname(nickname);
}

void MessageHandler::_handleUSER(User& user, const Message& message, Server& server) {
    validateUSER(user, message);
    std::string username = message.getParams()[0];
    user.setUsername(username);
}


void MessageHandler::_handlePING(User& user, const Message& message, Server& server) {
    validatePING(user, message);
    user.sendMessage("PONG " + message.getParams()[0]);
}

void MessageHandler::_handleJOIN(User& user, const Message& message, Server& server) {
    validateJOIN(user, message);
    std::map<std::string, Channel>& channelMap = server.getChannelMap();
    std::vector<std::string> channelNames = Utils::split(message.getParams()[0], ',');
    std::vector<std::string> keys;
    if (message.getParams().size() >= 2)
        std::vector<std::string> keys = Utils::split(message.getParams()[1], ',');
    while(!channelNames.empty()) {
        std::string currentChannel = channelNames.front();
        std::map<std::string, Channel>::iterator it = channelMap.find(currentChannel);
        if (it == channelMap.end()) {
            it = channelMap.insert(std::make_pair(currentChannel, Channel(currentChannel, server.getLogger()))).first;
            server.getLogger().log(INFO, "Channel created: #" + currentChannel);
        }
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

void MessageHandler::_handlePART(User& user, const Message& message, Server& server) {
    if (!_validatePART(message))
        throw std::invalid_argument("Wrong command format for PART");  
    if (!_isRegistered(user, message.getCommand(), server)) return;

    std::vector<std::string> channelNames = Utils::split(message.getParams()[0], ',');
    std::map<std::string, Channel>& channelMap = server.getChannelMap();

    while(!channelNames.empty()) {
        std::string currentChannel = channelNames.front();
        std::map<std::string, Channel>::iterator it = channelMap.find(currentChannel);
        if (it == channelMap.end())
            server.getLogger().log(WARNING, " Channel does not exist: " + currentChannel);
        else {
            std::string reason = message.getTrailing().empty() ? "Leaving" : message.getParams()[1];
            it->second.removeUser(user, reason);
            if (it->second.getMembers().empty()) {
                channelMap.erase(it);
                server.getLogger().log(INFO, "Channel removed: " + currentChannel);
            }
        }
        channelNames.erase(channelNames.begin());
    }
}

void MessageHandler::_handleKICK(User& user, const Message& message, Server& server) {
    if (!_validateKICK(message)) 
        throw std::invalid_argument("Wrong command format for KICK");
    if (!_isRegistered(user, message.getCommand(), server)) return;

        std::string channelName = message.getParams()[0];
        std::string targetName = message.getParams()[1];
        std::string reason = message.getTrailing().empty() ? "" : message.getTrailing();

        std::map<std::string, Channel>::iterator itChannel = server.getChannelMap().find(channelName);
        if (itChannel == server.getChannelMap().end())
            server.getLogger().log(WARNING, " Channel does not exist: " + channelName);

        itChannel->second.kickUser(user, targetName, reason);
}

void MessageHandler::_handleINVITE(User& user, const Message& message, Server& server) {
    if (!_validateINVITE(message)) 
        throw std::invalid_argument("Wrong command format for INVITE");
    if (!_isRegistered(user, message.getCommand(), server)) return;

        std::string nickName = message.getParams()[0];
        std::string channelName = message.getParams()[1];
        std::map<std::string, Channel>::iterator it = server.getChannelMap().find(channelName);
        if (it == server.getChannelMap().end())
            server.getLogger().log(WARNING, " Channel does not exist: " + channelName);
        it->second.inviteUser(user, nickName, server.getUserMap());
}

void MessageHandler::_handleTOPIC(User& user, const Message& message, Server& server) {
    if (!_validateTOPIC(message)) 
        throw std::invalid_argument("Wrong command format for TOPIC");
    if (!_isRegistered(user, message.getCommand(), server)) return;

        std::string channelName = message.getParams()[0];
        std::string topicName = message.getParams().size() == 2 ? message.getParams()[1] : "";

        std::map<std::string, Channel>::iterator it = server.getChannelMap().find(channelName);
        if (it == server.getChannelMap().end())
            server.getLogger().log(WARNING, " Channel does not exist: " + channelName);
        it->second.setTopic(user, topicName);
}

void MessageHandler::_handleMODE(User& user, const Message& message, Server& server) {
    if (!_validateMODE(message)) 
        throw std::invalid_argument("Wrong command format for MODE");
    if (!_isRegistered(user, message.getCommand(), server)) return;

        std::vector<std::string> paramsVec = message.getParams();
        if (paramsVec[1] == "+i") {
            user.sendMessage("MODE " + paramsVec[0] + " +i");
            return;
        }
        std::string channelName = paramsVec[0];

        std::map<std::string, Channel>::iterator it = server.getChannelMap().find(channelName);
        if (it == server.getChannelMap().end())
            server.getLogger().log(WARNING, " Channel does not exist: " + channelName);
        paramsVec.erase(paramsVec.begin());        
        it->second.changeMode(user, paramsVec);
}

void MessageHandler::validateCAP(const Message& message) {
    if (message.getParams().size() == 1 && (message.getParams()[0] == "LS" || message.getParams()[0] == "END"))
        return;
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validatePASS(User& user, const Message& message) {
    if (message.getParams().size() == 1) 
        return;
    user.sendMessage(ERR_NEEDMOREPARAMS +  " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + message.getCommand() +  " :Not enough parameter");
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validateNICK(User& user, const Message& message) {
     if (message.getParams().size() == 1) 
        return;
    user.sendMessage(ERR_NEEDMOREPARAMS + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + message.getCommand() +  " :Not enough parameter");
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validateUSER(User& user, const Message& message) {
    if (message.getParams().size() == 3 && !message.getTrailing().empty())
        return;
    user.sendMessage(ERR_NEEDMOREPARAMS + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + message.getCommand() +  " :Not enough parameter");
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validatePING(User& user, const Message& message) {
    if (!message.getParams().empty())
        return;
    user.sendMessage(ERR_NOORIGIN + " :No origin specified");
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validateJOIN(User& user, const Message& message) {
    if (!user.isRegistered()) {
        user.sendMessage(ERR_NOTREGISTERED + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + message.getCommand() + " :You have not registered");
        throw std::invalid_argument("User not registered.");
    }
    if (message.getParams().size() == 0) {
        user.sendMessage(ERR_NEEDMOREPARAMS + " " + user.getNickname().empty() ? "*" : user.getNickname() + " " + message.getCommand() + " :Not enough parameters");
        throw std::invalid_argument("Wrong command format");
    }
}

bool MessageHandler::_validatePART(const Message& message) {
    return (message.getParams().size() == 1);
}

bool MessageHandler::_validateKICK(const Message& message) {
    return (message.getParams().size() == 2);
}

bool MessageHandler::_validateINVITE(const Message& message) {
    return (message.getParams().size() == 2 && message.getTrailing().empty());
}

bool MessageHandler::_validateTOPIC(const Message& message) {
    return (message.getParams().size() == 1 && message.getParams().size() == 2 && message.getTrailing().empty());
}

bool MessageHandler::_validateMODE(const Message& message) {
    return (message.getParams().size() == 2 && message.getTrailing().empty());
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