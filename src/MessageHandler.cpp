#include "MessageHandler.hpp"

void MessageHandler::validateAndDispatch(int clientFd, const Message& message, Server& server) {
    if (message.getCommand() == "CAP") {
        if (message.getParams()[0] != "LS")
            throw std::invalid_argument("Wrong command format for CAP");
        server.getLogger().log(DEBUG, "CAP Command format validated");
        _handleCAP(clientFd, message, server);
    }
    else if (message.getCommand() == "PASS") {
        if (message.getParams().size() != 1 || !message.getTrailing().empty())
            throw std::invalid_argument("Wrong command format for PASS");
        server.getLogger().log(DEBUG, "PASS Command format validated");
        _handlePASS(clientFd, message, server);
    }
    else if (message.getCommand() == "NICK") {
        if (message.getParams().size() != 1 || !message.getTrailing().empty())
            throw std::invalid_argument("Wrong command format for NICK");
        server.getLogger().log(DEBUG, "NICK Command format validated");
        _handleNICK(clientFd, message, server);
    }
    else if (message.getCommand() == "USER") {
        if (message.getParams().size() != 3 || message.getTrailing().empty())
            throw std::invalid_argument("Wrong command format for USER");
        server.getLogger().log(DEBUG, "USER Command format validated");
    }
    else
        throw std::invalid_argument("Unknown command: " + message.getCommand());
}

void MessageHandler::_handleCAP(int clientFd, const Message& message, Server& server) {
    std::map<int, User>::iterator it = server.getUserMap().find(clientFd);
    if (it == server.getUserMap().end())
            throw::std::runtime_error("Could not find client");
    if (message.getParams()[0] == "LS")
        it->second.sendMessage("CAP * LS :");
    else if (message.getParams()[0] == "END")
        server.getLogger().log(DEBUG, "CAP END aknowledged");
    else
        throw::std::invalid_argument("Wrong CAP parameter");
    server.getLogger().log(DEBUG, "CAP command handled");
}

void MessageHandler::_handlePASS(int clientFd, const Message& message, Server& server) {
    std::map<int, User>::iterator it = server.getUserMap().find(clientFd);
    if (it == server.getUserMap().end())
            throw::std::runtime_error("Could not find client");
    
    if (message.getParams()[0] == server.getPassword())
                it->second.authenticate();
    else {
        it->second.sendMessage("464 - INCORRECT PASSWORD");
        server.getLogger().log(WARNING, "Wrong Password Received");
    }
    server.getLogger().log(DEBUG, "PASS command handled");
}

void MessageHandler::_handleNICK(int clientFd, const Message& message, Server& server) {
    std::map<int, User>::iterator it = server.getUserMap().find(clientFd);
    if (it == server.getUserMap().end())
            throw::std::runtime_error("Could not find client");
    it->second.setNickname(Utils::toString(message.getParams()[0]));
    server.getLogger().log(DEBUG, "NICK command handled");
}
