#include "MessageHandler.hpp"
#include "Errors.hpp"

std::map<std::string, MessageHandler::CommandHandler> MessageHandler::_cmdHandlers;

void MessageHandler::initCmdHandlers() {
    if (_cmdHandlers.empty()) {
        MessageHandler::_cmdHandlers.insert(std::make_pair("CAP", &MessageHandler::handleCAP));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PASS", &MessageHandler::handlePASS));
        MessageHandler::_cmdHandlers.insert(std::make_pair("NICK", &MessageHandler::handleNICK));
        MessageHandler::_cmdHandlers.insert(std::make_pair("USER", &MessageHandler::handleUSER));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PING", &MessageHandler::handlePING));
        MessageHandler::_cmdHandlers.insert(std::make_pair("JOIN", &MessageHandler::handleJOIN));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PART", &MessageHandler::handlePART));
        MessageHandler::_cmdHandlers.insert(std::make_pair("KICK", &MessageHandler::handleKICK));
        MessageHandler::_cmdHandlers.insert(std::make_pair("INVITE", &MessageHandler::handleINVITE));
        MessageHandler::_cmdHandlers.insert(std::make_pair("TOPIC", &MessageHandler::handleTOPIC));
        MessageHandler::_cmdHandlers.insert(std::make_pair("MODE", &MessageHandler::handleMODE));
        MessageHandler::_cmdHandlers.insert(std::make_pair("PRIVMSG", &MessageHandler::handlePRIVMSG));
        MessageHandler::_cmdHandlers.insert(std::make_pair("MOTD", &MessageHandler::handleMOTD));
        MessageHandler::_cmdHandlers.insert(std::make_pair("INFO", &MessageHandler::handleINFO));
    }
}

void MessageHandler::validateAndDispatch(User& user, const Message& message, Server& server) {
    initCmdHandlers();
    std::map<std::string, CommandHandler>::iterator itCmd = _cmdHandlers.find(message.getCommand());
    if (itCmd != _cmdHandlers.end())
        (itCmd->second)(user, message, server);
    else
        throw std::invalid_argument("Invalid Command: " + message.getCommand());
}

void MessageHandler::handleCAP(User& user, const Message& message, Server& server) {
    validateCAP(message);
    if (message.getParams()[0] == "LS")
        user.sendMessage("CAP * LS :");
    else if (message.getParams()[0] == "END")
        server.getLogger().log(DEBUG, "CAP END aknowledged");
}

void MessageHandler::handlePASS(User& user, const Message& message, Server& server) {    
    validatePASS(user, message);
    std::string password = message.getParams()[0];
    if (password != server.getPassword()) {
        user.sendErrorMessage(ERR_PASSWDMISMATCH, user, ":Password incorrect");
        throw std::invalid_argument("Incorrect password provided: " + password);
    }
    user.authenticate();
}

void MessageHandler::handleNICK(User& user, const Message& message, Server& server) {
    validateNICK(user, message);
    std::string nickname = message.getParams()[0];
    std::map<int, User>::iterator it = server.getUserMap().begin();
    while (it != server.getUserMap().end()) {
        if (it->second.getNickname() == nickname) {
            user.sendErrorMessage(ERR_NICKNAMEINUSE, user, nickname + " :Nickname is already in use");
            throw std::invalid_argument("Nickname already exists: " + nickname);
        }
        it++;
    }
    user.setNickname(nickname);
}

void MessageHandler::handleUSER(User& user, const Message& message, Server& server) {
    validateUSER(user, message);
    std::string username = message.getParams()[0];
    std::string hostname = message.getParams()[1];
    user.setUsername(username);
    user.setHostname(hostname);
    (void)server;
}


void MessageHandler::handlePING(User& user, const Message& message, Server& server) {
    validatePING(user, message);
    user.sendMessage("PONG " + message.getParams()[0]);
    (void)server;
}

void MessageHandler::handleMOTD(User& user, const Message& message, Server& server) {
    user.sendMOTD(server);
    (void)message;
}

void MessageHandler::handleINFO(User& user, const Message& message, Server& server) {
    user.sendINFO(server);
    (void)message;
}

void MessageHandler::handleJOIN(User& user, const Message& message, Server& server) {
    validateJOIN(user, message);
    std::map<std::string, Channel>& channelMap = server.getChannelMap();
    std::vector<std::string> channelNames = Utils::split(message.getParams()[0], ',');
    std::vector<std::string> keys = message.getParams().size() >= 2 ? Utils::split(message.getParams()[1], ',') : std::vector<std::string>() ;
    while(!channelNames.empty()) {
        std::string currentChannel =
         channelNames.front();
        std::map<std::string, Channel>::iterator it = channelMap.find(currentChannel);
        if (it == channelMap.end()) {
            if (currentChannel.size() >= 2 && (currentChannel[0] == '#' || currentChannel[0] == '&'))
                it = channelMap.insert(std::make_pair(currentChannel, Channel(currentChannel, server.getLogger()))).first;
            else {
                user.sendErrorMessage(ERR_NOSUCHCHANNEL, user, currentChannel + " :No such channel");
                throw std::invalid_argument("No such channel");
            }
        }
        else {
            if (it->second.isProtected()) {
                if (keys.empty()) {
                    user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, " MODE :Not enough parameters");
                    throw std::invalid_argument("Missing the new channel password");
                }
                it->second.addUser(user, keys.front());
                keys.erase(keys.begin());
                channelNames.erase(channelNames.begin());
                continue;
            }
        }
        it->second.addUser(user);
        channelNames.erase(channelNames.begin());
    }
}

void MessageHandler::handleINVITE(User& user, const Message& message, Server& server) {
    validateINVITE(user, message);
    std::string nickName = message.getParams()[0];
    std::string channelName = message.getParams()[1];
    std::map<std::string, Channel>::iterator it = server.getChannelMap().find(channelName);
    if (it == server.getChannelMap().end()) {
        user.sendErrorMessage(ERR_NOSUCHCHANNEL, user, channelName + " :No such channel");
        throw std::invalid_argument("Channel does not exist");
    }
    it->second.inviteUser(user, nickName, server.getUserMap());
}

void MessageHandler::handlePART(User& user, const Message& message, Server& server) {
    validatePART(user, message);
    if (message.getParams().empty()) {
        user.leaveAllChannels(server.getChannelMap());
        return;
    }
    std::vector<std::string> channelNames = Utils::split(message.getParams()[0], ',');
    std::string reason = message.getTrailing().empty() ? "No reason provided" : message.getParams()[1];
    std::map<std::string, Channel>& channelMap = server.getChannelMap();
    while(!channelNames.empty()) {
        std::string currentChannel = channelNames.front();
        std::map<std::string, Channel>::iterator it = channelMap.find(currentChannel);
        if (it == channelMap.end()) {
            user.sendErrorMessage(ERR_NOSUCHCHANNEL, user, currentChannel + " :No such channel");
            server.getLogger().log(WARNING, "Channel does not exist: " + currentChannel);
        }
        else {
            it->second.partUser(user, reason);
            if (it->second.getMembers().empty()) {
                channelMap.erase(it);
                server.getLogger().log(INFO, "Empty channel removed: " + currentChannel);
            }
        }
        channelNames.erase(channelNames.begin());
    }
}

void MessageHandler::handleTOPIC(User& user, const Message& message, Server& server) {
    validateTOPIC(user, message);
    std::string channelName = message.getParams()[0];
    std::string topicName = message.getParams().size() == 2 ? message.getParams()[1] : std::string();

    std::map<std::string, Channel>::iterator it = server.getChannelMap().find(channelName);
    if (it == server.getChannelMap().end()) {
        user.sendErrorMessage(ERR_NOSUCHCHANNEL, user, channelName + " :No such channel");
        throw std::invalid_argument("Channel does not exist");
    }
    it->second.setTopic(user, topicName);
}

void MessageHandler::handleMODE(User& user, const Message& message, Server& server) {
    validateMODE(user, message);
    std::vector<std::string> paramsVec = message.getParams();
    if (paramsVec[0][0] == '#' || paramsVec[0][0] == '&') {
        std::string channelName = paramsVec[0];
        std::map<std::string, Channel>::iterator it = server.getChannelMap().find(channelName);
        if (it == server.getChannelMap().end()) {
            user.sendErrorMessage(ERR_NOSUCHCHANNEL, user, channelName + " :No such channel");
            throw std::invalid_argument("Channel does not exist");
        }
        paramsVec.erase(paramsVec.begin());
        if (paramsVec.empty())
            it->second.printMode(user);
        else
            it->second.changeMode(user, paramsVec);
    }
    else {
        std::string nickname = paramsVec[0];
        std::map<int, User>::iterator it = server.getUserMap().begin();
        while (it != server.getUserMap().end()) {
            if (it->second.getNickname() == nickname) {
                paramsVec.erase(paramsVec.begin());
                it->second.changeMode(paramsVec);
                return;
            }
            it++;
        }
        user.sendErrorMessage(ERR_NOSUCHNICK, user, nickname + " :No such nick/channel");
        throw std::invalid_argument("The target user does not exist");
        it++;
    }
}

void MessageHandler::handleKICK(User& user, const Message& message, Server& server) {
    validateKICK(user, message);
    std::string channelName = message.getParams()[0];
    std::string targetName = message.getParams()[1];
    std::string reason = message.getTrailing().empty() ? "No reason provided" : message.getTrailing();
    std::map<std::string, Channel>::iterator it = server.getChannelMap().find(channelName);
    if (it == server.getChannelMap().end()) {
        user.sendErrorMessage(ERR_NOSUCHCHANNEL, user, channelName + " :No such channel");
        throw std::invalid_argument("Channel does not exist");
    }
    it->second.kickUser(user, targetName, reason);
}

void MessageHandler::handlePRIVMSG(User& user, const Message& message, Server& server) {
    validatePRIVMSG(user, message);
    std::vector<std::string> params = Utils::split(message.getParams()[0], ',');
    for (size_t i = 0; i < params.size(); i++) {
        std::string currentParam = params[i];
        if (currentParam[0] == '#' || currentParam[0] == '&') {
            std::map<std::string, Channel>::iterator it = server.getChannelMap().find(currentParam);
            if (it == server.getChannelMap().end()) {
                user.sendErrorMessage(ERR_NOSUCHCHANNEL, user, currentParam + " :No such channel");
                server.getLogger().log(WARNING, "Channel " + currentParam + " does not exist");
                continue;
            }
            if (it->second.getMembers().find(user.getSocketFd()) == it->second.getMembers().end()) {
                user.sendErrorMessage(ERR_CANNOTSENDTOCHAN, user, it->first + " :Cannot send to channel");
                throw std::invalid_argument("The user is not part of the channel");
            }
            it->second.broadcast(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " PRIVMSG " + currentParam + " :" + message.getTrailing(), user.getSocketFd(), false);
        }
        else {
            std::map<int, User>::iterator it;
            for (it = server.getUserMap().begin(); it != server.getUserMap().end(); ++it) {
                if (it->second.getNickname() == currentParam)
                    break;
            }
            if (it == server.getUserMap().end()) {
                user.sendErrorMessage(ERR_NOSUCHNICK, user, currentParam + " :No such nick/channel");
                server.getLogger().log(WARNING, "User " + currentParam + " does not exist");
            }
            it->second.sendMessage(":" + user.getNickname() + "!" + user.getUsername() + "@" + user.getHostname() + " PRIVMSG :" + message.getTrailing(), false);
        }
    }
}

void MessageHandler::validateCAP(const Message& message) {
    if (message.getParams().size() == 1 && (message.getParams()[0] == "LS" || message.getParams()[0] == "END"))
        return;
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validatePASS(User& user, const Message& message) {
    if (message.getParams().size() == 1) 
        return;
    user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, message.getCommand() +  " :Not enough parameters");
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validateNICK(User& user, const Message& message) {
     if (message.getParams().size() == 1) 
        return;
    user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, message.getCommand() +  " :Not enough parameters");
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validateUSER(User& user, const Message& message) {
    if (message.getParams().size() >= 3)
        return;
    user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, message.getCommand() +  " :Not enough parameters");
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validatePING(User& user, const Message& message) {
    if (!message.getParams().empty())
        return;
    user.sendErrorMessage(ERR_NOORIGIN, user, "No origin specified");
    throw std::invalid_argument("Wrong command format");
}

void MessageHandler::validateJOIN(User& user, const Message& message) {
    if (!user.isRegistered()) {
        user.sendErrorMessage(ERR_NOTREGISTERED, user, message.getCommand() + " :You have not registered");
        throw std::invalid_argument("User not registered");
    }
    if (message.getParams().size() == 0) {
        user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, message.getCommand() + " :Not enough parameters");
        throw std::invalid_argument("Wrong command format");
    }
}

void MessageHandler::validateINVITE(User& user, const Message& message) {
    if (!user.isRegistered()) {
        user.sendErrorMessage(ERR_NOTREGISTERED, user, message.getCommand() + " :You have not registered");
        throw std::invalid_argument("User not registered");
    }
    if (message.getParams().size() < 2) {
        user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, message.getCommand() + " :Not enough parameters");
        throw std::invalid_argument("Wrong command format");
    }
}

void MessageHandler::validatePART(User& user, const Message& message) {
    if (!user.isRegistered()) {
        user.sendErrorMessage(ERR_NOTREGISTERED, user, message.getCommand() + " :You have not registered");
        throw std::invalid_argument("User not registered");
    }
}

void MessageHandler::validateKICK(User& user, const Message& message) {
    if (!user.isRegistered()) {
        user.sendErrorMessage(ERR_NOTREGISTERED, user, message.getCommand() + " :You have not registered");
        throw std::invalid_argument("User not registered");
    }
    if (message.getParams().size() < 2) {
        user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, message.getCommand() + " :Not enough parameters");
        throw std::invalid_argument("Wrong command format");
    }
}

void MessageHandler::validateTOPIC(User& user, const Message& message) {
    if (!user.isRegistered()) {
        user.sendErrorMessage(ERR_NOTREGISTERED, user, message.getCommand() + " :You have not registered");
        throw std::invalid_argument("User not registered");
    }
    if (message.getParams().size() == 0) {
        user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, message.getCommand() + " :Not enough parameters");
        throw std::invalid_argument("Wrong command format");
    }
}

void MessageHandler::validateMODE(User& user, const Message& message) {
    if (!user.isRegistered()) {
        user.sendErrorMessage(ERR_NOTREGISTERED, user, message.getCommand() + " :You have not registered");
        throw std::invalid_argument("User not registered.");
    }
    if (message.getParams().size() == 0) {
        user.sendErrorMessage(ERR_NEEDMOREPARAMS, user, message.getCommand() + " :Not enough parameters");
        throw std::invalid_argument("Wrong command format");
    }
}

void MessageHandler::validatePRIVMSG(User& user, const Message& message) {
    if (!user.isRegistered()) {
        user.sendErrorMessage(ERR_NOTREGISTERED, user, message.getCommand() + " :You have not registered");
        throw std::invalid_argument("User not registered");
    }
    if (message.getParams().size() == 0) {
        user.sendErrorMessage(ERR_NORECIPIENT, user, message.getCommand() + " :No recipient given");
        throw std::invalid_argument("Wrong command format");
    }
    if (message.getTrailing().empty()) {
        user.sendErrorMessage(ERR_NOTEXTTOSEND, user, message.getCommand() + " :No text to send");
        throw std::invalid_argument("Wrong command format");
    }
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