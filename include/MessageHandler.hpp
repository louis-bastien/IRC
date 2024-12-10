#pragma once
#include <string>
#include "Logger.hpp"
#include "Message.hpp"
#include "Server.hpp"

class MessageHandler {
    public:
        typedef void (*CommandHandler)(User&, const Message&, Server&);
        static void validateAndDispatch(int clientFd, const Message& message, Server& server);
    
    private:
        static std::map<std::string, CommandHandler> _cmdHandlers;
        
        static void _initCmdHandlers(void);
        static void _setupAuth(User& user, const Message& message, Server& server);
        static bool _isAuthenticated(User& user, const std::string& command, Server& server);
        static bool _isRegistered(User& user, const std::string& command, Server& server);

        static void _handleCAP(User& user, const Message& message, Server& server);
        static void _handlePASS(User& user, const Message& message, Server& server);
        static void _handleNICK(User& user, const Message& message, Server& server);
        static void _handleUSER(User& user, const Message& message, Server& server);
        static void _handlePING(User& user, const Message& message, Server& server);
        static void _handleJOIN(User& user, const Message& message, Server& server);

        static bool _validateCAP(const Message& message);
        static bool _validatePASS(const Message& message);
        static bool _validateNICK(const Message& message);
        static bool _validateUSER(const Message& message);
        static bool _validateJOIN(const Message& message);
        static bool _validateUSER(const Message& message);
};