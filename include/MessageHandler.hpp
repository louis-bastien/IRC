#pragma once
#include <string>
#include "Logger.hpp"
#include "Message.hpp"

class MessageHandler {
    public:
        static void validate(const Message& message, Logger& logger);
        static void dispatch(const Message& message, Logger& logger);
};