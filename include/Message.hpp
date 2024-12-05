#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iostream>


class Message {
    private:
        std::string _prefix;
        std::string _command;
        std::string _trailing;
        std::vector<std::string> _params;
        
        void parse(const std::string& rawMessage);
        void exctractPrefix(std::istringstream& ss);
        void exctractCommand(std::istringstream& ss);
    
    public:
        Message(const std::string& rawMessage);
};