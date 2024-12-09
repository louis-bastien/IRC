#include "Message.hpp"


Message::Message(const std::string& rawMessage) {
    parse(rawMessage);
}

void Message::parse(const std::string& rawMessage) {
    std::istringstream ss(rawMessage);

    if (rawMessage[0] == ':')
        exctractPrefix(ss);
    exctractCommand(ss);rawMessage
    exctractParams(ss);
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

void Message::exctractParams(std::istringstream& ss) {
    std::string token;
    
    while (ss >> token) {
        if (token[0] == ':') {
            _trailing = token.substr(1);
            break;
        }
        _params.push_back(token);
    }       
}

void Message::exctractTrailing(std::istringstream& ss) {
    _trailing += ss.str();
}

void Message::logMsg(Logger& logger) {
    logger.log(DEBUG, "#Parsed message content is:");
    logger.log(DEBUG, " #prefix=" + _prefix);
    logger.log(DEBUG," #command=" + _command);

    std::stringstream ss;
    std::vector<std::string>::iterator it;
    for (it = _params.begin(); it != _params.end(); it++) 
        ss << *it;
    logger.log(DEBUG," #params=" + ss.str());
    logger.log(DEBUG," #trailing=" + _trailing);    
}
