#include "Server.hpp"
#include "Message.hpp"
#include "MessageHandler.hpp"
#include "Errors.hpp"

//Satic pipe fds used for signal handling with Epoll. 
int Server::_pipeFd[2] = {-1, -1};

Server::Server(int port, std::string password, Logger& logger) : _port(port), _password(password), _logger(logger), _motd("Today is a good day!") {
    _logger.log(INFO, "starting server on port " + Utils::toString(_port) + " with password '" + _password + "'");
}

Server::~Server() {
    std::map<int, User>::iterator it;
    for (it = _userMap.begin(); it != _userMap.end(); it++) {
        if (close(it->first) < 0) 
            _logger.log(ERROR, "fd " + Utils::toString(it->first) + " failed to close");
        else
            _logger.log(DEBUG, "Client fd " + Utils::toString(it->first) + " closed");
    }

    if (close (_epollFd) < 0)
        _logger.log(ERROR, "Epoll fd failed to close");

    if (close (_serverFd) < 0)
        _logger.log(ERROR, "Server fd failed to close");
    
    if (close(_pipeFd[0]) < 0 || close(_pipeFd[1]) < 0)
        _logger.log(ERROR, "Failed to close pipe ends");

    _logger.log(DEBUG, "Server instance destroyed");
}

void Server::init() {
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(_port);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    _serverFd = socket(AF_INET, SOCK_STREAM, 0);

    if (_serverFd < 0) {
        _logger.log(ERROR, "Failed to create server socket");
        throw std::runtime_error("Failed to create server socket");
    }
    _logger.log(INFO, "Server socket successfully created");

    int opt = 1;
    if (setsockopt(_serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        _logger.log(ERROR, "Failed to set SO_REUSEADDR on server socket");
        throw std::runtime_error("Failed to set SO_REUSEADDR on server socket");
    }

    if (bind(_serverFd, (const sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        _logger.log(ERROR, "Failed to bind server socket");
        throw std::runtime_error("Failed to bind server socket");
    }
    _logger.log(INFO, "Server socket bound to port " + Utils::toString(_port));
    
    listen(_serverFd, MAX_CONNECTIONS);
    _logger.log(INFO, "Server listening for connections (max_connections=" + Utils::toString(MAX_CONNECTIONS) + ")");

    if (pipe(_pipeFd) < 0) {
        _logger.log(ERROR, "Failed to create pipe for signal handling");
        throw std::runtime_error("Failed to create pipe for signal handling");
    }

}

void Server::start(void) {
    epollInit();
    _logger.log(INFO, "Epoll instance created.");
    
    signal(SIGINT, signalHandler);
    epollAddFd(_pipeFd[0]);
    _logger.log(INFO, "Signal handler setup and read-end added to epoll instance.");
    _logger.log(INFO, "Waiting for events to occur...");

    while (true) {
        struct epoll_event events[MAX_EVENTS];

        int n = epoll_wait(_epollFd, events, MAX_EVENTS, -1);
        _logger.log(DEBUG, "Epoll_wait returned " + Utils::toString(n) + " events");

        if (n < 0) {
            if (errno == EINTR) {
            //If a SIGINT is received, continue the loop to detect the read event from the signal pipe
                _logger.log(WARNING, "Epoll_wait interrupted by signal");
                continue;
            }
            _logger.log(ERROR, "Epoll_wait failed");
            throw std::runtime_error("Epoll_wait failed");
        }
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == _serverFd)
                acceptConnection();
            else if (events[i].data.fd == _pipeFd[0]) {
                char buf[1];
                read(_pipeFd[0], &buf, 1);
                _logger.log(INFO, "Signal received. Shutting down server...");
                return;
            }
            else if (events[i].events & EPOLLIN)
                handleReadEvent(events[i].data.fd);
        }
    }
}

void Server::signalHandler(int sign) {
    if (sign == SIGINT || sign == SIGQUIT)
        write(_pipeFd[1], "0", 1);
}

void Server::epollInit(void) {
    _epollFd = epoll_create1(0);
    if (_epollFd < 0) {
        _logger.log(ERROR, "Failed to create epoll instance");
        throw std::runtime_error("Failed create epoll instance");
    }
    epollAddFd(_serverFd);
}

void Server::acceptConnection(void) {
    struct sockaddr clientAddr;
    socklen_t clientAddrSize;

    int clientFd = accept(_serverFd, &clientAddr, &clientAddrSize);
    if (clientFd < 0) {
        _logger.log(WARNING, "Failed to accept new client fd=" + Utils::toString(clientFd));
        return ;
    }
    _logger.log(INFO, "Accepted new client fd=" + Utils::toString(clientFd));
    
    struct sockaddr_in clientAddrIn = *reinterpret_cast<struct sockaddr_in*>(&clientAddr);
    std::string ipAddrStr(inet_ntoa(clientAddrIn.sin_addr));
    std::string portStr(Utils::toString(ntohs(clientAddrIn.sin_port)));
    _logger.log(DEBUG, "IP [" + ipAddrStr + "] Port [" + portStr +"]");

    try {
        User newUser(clientFd, _logger);
        _userMap.insert(std::make_pair(clientFd, newUser));
        epollAddFd(clientFd);
    } catch (const std::exception &e) {
        _logger.log(ERROR, "Error adding new user : " + std::string(e.what()));
        closeClient(clientFd);
    }
}

void Server::handleReadEvent(int eventFd) {
    char buffer[BUFFER_SIZE];
    std::string rawMessage;

    while (true) {
        int bytes_read = read(eventFd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            closeClient(eventFd);
            _logger.log(ERROR, "Failed to read from client");
            return;
        }
        else if (bytes_read == 0) {
            if (rawMessage.size() > 0)
                _logger.log(INFO, "Incomplete message received from client (no CR-LR): " + rawMessage);
            _logger.log(INFO, "Connection terminated by client");
            closeClient(eventFd);
            return;
        }
        else {
            rawMessage.append(buffer, bytes_read);
            if (rawMessage.find("\r\n") != std::string::npos)
                break;
        }
    }
    handleMessage(eventFd, rawMessage);
}

void Server::handleMessage(int clientFd, std::string& rawMessage) {
    std::map<int, User>::iterator it = _userMap.find(clientFd);
    if (it == _userMap.end())
            throw::std::runtime_error("Could not find client fd=" + Utils::toString(clientFd));
    User& user = it->second;
    if (rawMessage.size() > 512) {
        user.sendErrorMessage(ERR_INPUTTOOLONG, user, " :Input line was too long");
        _logger.log(WARNING, "Message too long: " + rawMessage);
        return;
    }
    try {
        Message msg(rawMessage);
        msg.logMsg(_logger);
        MessageHandler::validateAndDispatch(user, msg, *this);
        if (!user.isRegistered())
            user.doRegister(*this);
        _logger.log(DEBUG, "Command handled without error");
    }
    catch (std::exception &e) { 
        _logger.log(WARNING, "Error processing message: " + Utils::toString(e.what()));
    }
}

void Server::closeClient(int clientFd) {
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, NULL);
    _userMap.find(clientFd)->second.leaveAllChannels(getChannelMap());
    _userMap.erase(clientFd);
    _logger.log(INFO, "Closed client (fd=" + Utils::toString(clientFd) + ") successfully");
}

void Server::epollAddFd(int newFd) {
    struct epoll_event epev;
    epev.events = EPOLLIN;
    epev.data.fd  = newFd;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, newFd, &epev) < 0) {        
        _logger.log(ERROR, "Failed to add server event to epoll instance");
        throw std::runtime_error("Failed to add server event to epoll instance");
    }
    _logger.log(DEBUG, "Read event (fd=" + Utils::toString(newFd) + ") added to epoll instance");
}

Logger& Server::getLogger(void) const {
    return (_logger);
}

const std::string& Server::getPassword(void) const {
    return (_password);
}

const std::string& Server::getMOTD(void) const {
    return (_motd);
}

std::map<int, User>& Server::getUserMap(void) {
    return (_userMap);
}

std::map<std::string, Channel>& Server::getChannelMap(void) {
    return (_channelMap);
}
