#pragma once
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <map>

#include <Logger.hpp>
#include <User.hpp>
//#include <Channel.hpp>

#define BUFFER_SIZE 512
#define MAX_EVENTS 24
#define MAX_CONNECTIONS 100
#define MAX_CLIENTS 10

class User;
class Channel;

class Server {
    private: 
        int _port;
        std::string _password;
        int _serverFd;
        int _epollFd;
        Logger& _logger;
        std::map<int, User> _userMap;
        std::map<std::string, Channel> _channelMap;

        void epollInit(void);
        void acceptConnection(void);
        void handleReadEvent(int eventFd);
        void epollAddFd(int fd);
        void handleMessage(std::string message);

    public:
        Server(int port, std::string password, Logger& logger);
        ~Server();

        const std::string& getPassword(void) const;

        void init(void);
        void start(void);
        void closeClient(int clientFd);
};



