#pragma once
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>

#include <user.hpp>
#include <channel.hpp>

#define BUFFER_SIZE 1024
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

        std::map<int, User> _userMap;
        std::map<std::string, Channel> _channelMap;

        void epollInit(void);
        void acceptConnection(void);
        void handleReadEvent(int eventFd);

    public:
        server(int port, std::string password);
        ~server();

        const std::string& getPassword(void) const;

        void init(void);
        void start(void);

        void endConnection(void);

        std::string readFromSocket(int socketFd) const;
        void writeToSocket(int socketFD, std::string message);
};



