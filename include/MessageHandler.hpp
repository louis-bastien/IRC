#pragma once
#include <string>
#include "Logger.hpp"
#include "Message.hpp"
#include "Server.hpp"

class MessageHandler {
    private:
        static void _handleCAP(int clientFd, const Message& message, Server& server);
        static void _handlePASS(int clientFd, const Message& message, Server& server);
        static void _handleNICK(int clientFd, const Message& message, Server& server);
        
    public:
        static void validateAndDispatch(int clientFd, const Message& message, Server& server);
};