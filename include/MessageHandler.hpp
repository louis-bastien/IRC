#pragma once
#include <string>
#include "Logger.hpp"
#include "Message.hpp"
#include "Server.hpp"

class MessageHandler {
    public:
        typedef void (*CommandHandler)(User&, const Message&, Server&);
        static void validateAndDispatch(User& user, const Message& message, Server& server);
    
    private:
        static std::map<std::string, CommandHandler> _cmdHandlers;
        
        static void initCmdHandlers(void);

        static void handleCAP(User& user, const Message& message, Server& server);
        static void handlePASS(User& user, const Message& message, Server& server);
        static void handleNICK(User& user, const Message& message, Server& server);
        static void handleUSER(User& user, const Message& message, Server& server);
        static void handlePING(User& user, const Message& message, Server& server);
        static void handleJOIN(User& user, const Message& message, Server& server);
        static void handlePART(User& user, const Message& message, Server& server);
        static void handleKICK(User& user, const Message& message, Server& server);
        static void handleINVITE(User& user, const Message& message, Server& server);
        static void handleTOPIC(User& user, const Message& message, Server& server);
        static void handleMODE(User& user, const Message& message, Server& server);
        static void handlePRIVMSG(User& user, const Message& message, Server& server);
        static void handleINFO(User& user, const Message& message, Server& server);
        static void handleMOTD(User& user, const Message& message, Server& server);
        static void handleWHO(User& user, const Message& message, Server& server);

        static void validateCAP(const Message& message);
        static void validatePASS(User& user, const Message& message);
        static void validateNICK(User& user, const Message& message);
        static void validateUSER(User& user, const Message& message);
        static void validatePING(User& user, const Message& message);
        static void validateJOIN(User& user, const Message& message);
        static void validateINVITE(User& user, const Message& message);
        static void validatePART(User& user, const Message& message);
        static void validateKICK(User& user, const Message& message);
        static void validateTOPIC(User& user, const Message& message);
        static void validateMODE(User& user, const Message& message);
        static void validatePRIVMSG(User& user, const Message& message);
};