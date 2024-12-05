#include "Server.hpp"

Server::Server(int port, std::string password, Logger& logger) : _port(port), _password(password), _logger(logger) {}

Server::~Server() {}

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

    if (bind(_serverFd, (const sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        _logger.log(ERROR, "Failed to bind server socket");
        throw std::runtime_error("Failed to bind server socket");
    }
    _logger.log(INFO, "Server socket bound to port " + std::to_string(_port));
    
    listen(_serverFd, MAX_CONNECTIONS);
    _logger.log(INFO, "Server listening for connections (max_connections=" + std::to_string(MAX_CONNECTIONS) + ")");
}

void Server::start(void) {
    epollInit();
    _logger.log(INFO, "Epoll instance created successfully");

    while (true) {
        struct epoll_event events[MAX_EVENTS];

        int n = epoll_wait(_epollFd, events, MAX_EVENTS, -1);
        _logger.log(DEBUG, "Epoll_wait returned " + std::to_string(n) + " events");

        if (n < 0) {
            _logger.log(ERROR, "Epoll_wait failed");
            throw std::runtime_error("Epoll_wait failed");
        }
        for (int i = 0; i < n; i++) {
            if (events[i].data.fd == _serverFd)
                acceptConnection();
            else if (events[i].events & EPOLLIN)
                handleReadEvent(events[i].data.fd);
        }
    }
}

void Server::epollInit(void) {
    _epollFd = epoll_create1(0);
    if (_epollFd < 0) {
        _logger.log(ERROR, "Failed to create epoll instance");
        throw std::runtime_error("Failed create epoll instance");
    }
    epollAddFd(_serverFd);
}

//We need to work on how to add/handle new users
//I assume User can be instantiated with only its associated socket fd;
//We also need to figur out what to do with the client's address
//RFC says:
//  all servers must have the
//  following information about all clients: the real name of the host
//  that the client is running on, the username of the client on that
//  host, and the server to which the client is connected.
void Server::acceptConnection(void) {
    struct sockaddr clientAddr;
    socklen_t clientAddrSize;

    int clientFd = accept(_serverFd, &clientAddr, &clientAddrSize);
    if (clientFd < 0) {
        _logger.log(WARNING, "Failed to accept new client");
        return ;
    }
    _logger.log(INFO, "Accepted new client with fd " + std::to_string(clientFd));
    
    try {
        User newUser(clientFd);
        _userMap.insert(std::make_pair(clientFd, newUser));
        epollAddFd(clientFd);
    } catch (const std::exception &e) {
        _logger.log(ERROR, "Error adding new user : " + std::string(e.what()));
        close(clientFd);
    }
}

//Get message only. Parsing/execution done later
void Server::handleReadEvent(int eventFd) {
    char buffer[BUFFER_SIZE];
    std::string message;

    while (true) {
        int bytes_read = read(eventFd, buffer, sizeof(buffer));
        if (bytes_read < 0) {
            closeClient(eventFd);
            throw std::runtime_error("Failed to read from client fd");
        }
        else if (bytes_read == 0)
            closeClient(eventFd);
        else {
            message.append(buffer, bytes_read);
            if (message.find("\r\n") != std::string::npos)
                break;
        }
    }
    handleMessage(message);
}

void Server::closeClient(int clientFd) {
    epoll_ctl(_epollFd, EPOLL_CTL_DEL, clientFd, NULL);
    s
    _userMap.erase(clientFd);
    if (close(clientFd) < 0)
        throw std::runtime_error("Failed to close client fd");
    //also need to remove the user from all channels
}

void Server::epollAddFd(int newFd) {
    struct epoll_event epev;
    epev.events = EPOLLIN;
    epev.data.fd  = newFd;
    if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, newFd, &epev) < 0) {        
        _logger.log(ERROR, "Failed to add server event to epoll instance");
        throw std::runtime_error("Failed to add server event to epoll instance");
    }
    _logger.log(DEBUG, "Read event for fd" + std::to_string(newFd) + "added to epoll instance");
}

const std::string& Server::getPassword(void) const {
    return (_password);
}
