#include "server.hpp"

Server::Server(int port, std::string password) : _port(port), _password(password) {}

void Server::init(void) {
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(1234);
    serv_addr.sin_addr.s_addr = INADDR_ANY;

    _serverFd = socket(AF_INET, SOCK_STREAM, 0);
    if (serv_fd < 0)
        throw std::runtime_error("Failed to create server socket")

    if (bind(serv_fd, (const sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
        throw std::runtime_error("Failed to bind server socket")

    listen(_serverFd, MAX_CONNECTIONS)
}

void Server::start(void) {
    epollInit();
    while (true) {
        struct epoll_event events[MAX_EVENTS];
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n < 0)
            throw std::runtime_error("Epoll_wait failed")
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
    if (_epollFd < 0)
        throw std::runtime_error("Failed creat epoll instance");
    epollAddFd(_serverFd);
}

//We need to work on how to add/handle new users
//I assume User can be instantiated with only its associated socket fd;
void Server::acceptConnection(void) {
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize;

    int clientFd = accept(_serverFd, &clientAddr, &clientAddrSize);
    if (client_fd < 0)
        throw std::runtime_error("Failed to accept new client");
    User NewUser(clientFd);
    _userMap.insert(clientFd, newUser);
    epollAddFd(clientFd);
}

void Server::handleReadEvent(int eventFd) {
    char buffer[BUFFER_SIZE];
    std::string message;

    int bytes_read = read(eventFd, buffer, sizeof(buffer));
    while (true) {
        if (bytes_read < 0) {
            closeClient(eventFd);
            throw std::runtime_error("Failed to read from client fd");
        }
        if (bytes_read == 0)
            closeClient(eventFd);
        else {
            message.append(buffer, bytes_read);
            if (message.find('\n') != std::string::npos)
                break;
        }
    }
}

void Server::closeClient(int clientFd) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, NULL);
    _userMap.erase(clientFd);
    if (close(clientFd) < 0)
        throw std::runtime_error("Failed to close client fd");
    //also need to remove the user from all channels
}

void Server::epollAddFd(newFd) {
    struct epoll_event epev;
    epev.events = EPOLLIN;
    epev.data.fd  = newFd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, newFd, &epev) < 0)
        throw std::runtime_error("Failed to add server event to epoll instance");

}

const std::string& Server::getPassword(void) const {
    return (_password);
}
