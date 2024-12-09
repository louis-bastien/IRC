#include "MessageHandler.hpp"

void MessageHandler::validate(const Message& message, Logger& logger) {
    if (message.getCommand() == "PASS") {
        if (message.getParams().size() != 1 || !message.getTrailing().empty())
            throw std::invalid_argument("Wrong command format for PASS");
        logger.log(DEBUG, "PASS Command format validated");
    }
    if (message.getCommand() == "NICK") {
        if (message.getParams().size() != 1 || !message.getTrailing().empty())
            throw std::invalid_argument("Wrong command format for NICK");
        logger.log(DEBUG, "NICK Command format validated");
    }
    if (message.getCommand() == "USER") {
        if (message.getParams().size() != 3 || message.getTrailing().empty())
            throw std::invalid_argument("Wrong command format for USER");
        logger.log(DEBUG, "USER Command format validated");
    }   
}