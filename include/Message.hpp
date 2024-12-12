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
        std::vector<std::string> _params;
        std::string _trailing;
        
        void parse(const std::string& rawMessage);
        void extractPrefix(std::istringstream& ss);
        void extractCommand(std::istringstream& ss);
        void extractParamsAndTrailing(std::istringstream& ss);
    
    public:
        Message(const std::string& rawMessage);

        const std::string& getPrefix(void) const;
        const std::string& getCommand(void) const;
        const std::string& getTrailing(void) const;
        const std::vector<std::string>& getParams(void) const; 

        void logMsg(Logger& logger);
};