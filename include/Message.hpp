#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "Logger.hpp"


class Message {
    private:
        std::string _prefix;
        std::string _command;
        std::string _trailing;
        std::vector<std::string> _params;
        
        void parse(const std::string& rawMessage);
        void exctractPrefix(std::istringstream& ss);
        void exctractCommand(std::istringstream& ss);
        void exctractParams(std::istringstream& ss);
        void exctractTrailing(std::istringstream& ss);
    
    public:
        Message(const std::string& rawMessage);
        void logMsg(Logger& logger);
};