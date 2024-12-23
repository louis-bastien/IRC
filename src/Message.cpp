#include "Message.hpp"


Message::Message(const std::string& rawMessage) {
    parse(rawMessage);
}

void Message::parse(const std::string& rawMessage) {
    if (rawMessage.empty())
        throw std::invalid_argument("Raw message cannot be empty");

    std::istringstream ss(rawMessage);
    if (rawMessage[0] == ':')
        extractPrefix(ss);
    extractCommand(ss);
    extractParamsAndTrailing(ss);
}

void Message::extractPrefix(std::istringstream& ss) {
    std::string token;
    
    ss >> token;
    _prefix = token.substr(1);
}

void Message::extractCommand(std::istringstream& ss) {
    std::string token;
    
    ss >> token;
    _command = token;

    if (_command.empty())
        throw std::invalid_argument("Command is missing");
}

void Message::extractParamsAndTrailing(std::istringstream& ss) {
    std::string token;
    
    while (ss >> token) {
        if (token[0] == ':') {
            std::getline(ss, _trailing);
            _trailing = token.substr(1) + _trailing;
            break;
        }
        _params.push_back(token);
    }       
}

const std::string& Message::getPrefix(void) const {
    return(_prefix);
}
const std::string& Message::getCommand(void) const {
    return(_command);    
}

const std::string& Message::getTrailing(void) const {
    return(_trailing);
}

const std::vector<std::string>& Message::getParams(void) const {
    return(_params);
}

void Message::logMsg(Logger& logger) {
    logger.log(DEBUG, "#Parsed message content is:");
    logger.log(DEBUG, " #prefix=" + _prefix);
    logger.log(DEBUG," #command=" + _command);

    std::stringstream ss;
    std::vector<std::string>::iterator it;
    for (it = _params.begin(); it != _params.end(); it++) 
        ss << *it << " ";
    logger.log(DEBUG," #params=" + ss.str());
    logger.log(DEBUG," #trailing=" + _trailing);    
}

