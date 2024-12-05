#include "Message.hpp"


Message::Message(const std::string& rawMessage) {
    parse(rawMessage);
}

void Message::parse(const std::string& rawMessage) {
    std::istringstream ss(rawMessage);

    if (rawMessage[0] == ':')
        exctractPrefix(ss);
    exctractCommand{}
}

void Message::exctractPrefix(std::istringstream& ss) {
    std::string token;
    
    ss >> token;
    _prefix = token.substr(1);
}

void Message::exctractCommand(std::istringstream& ss) {
    std::string token;
    
    ss >> token;
    _command = token;
}






