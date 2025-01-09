#pragma once
#include <string>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <map>
#include <csignal>
#include <fcntl.h>

#include <Logger.hpp>
#include <User.hpp>
#include <Utils.hpp>
//#include <Channel.hpp>

#define BUFFER_SIZE 1 //Do not increase to avoid command overflowing
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
        std::string _motd;
        static int _pipeFd[2];
        std::map<int, User> _userMap;
        std::map<std::string, Channel> _channelMap;
        std::map<int, std::string> clientBuffers; 

        void epollInit(void);
        void acceptConnection(void);
        void handleReadEvent(int eventFd);
        void epollAddFd(int fd);
        void handleMessage(int clientFd, std::string& rawMessage);

    public:
        Server(int port, std::string password, Logger& logger);
        ~Server();

        const std::string& getPassword(void) const;
        const std::string& getMOTD(void) const;
        Logger& getLogger(void) const;
        std::map<int, User>& getUserMap(void);
        std::map<std::string, Channel>& getChannelMap(void);

        void init(void);
        void start(void);
        void closeClient(int clientFd);
        static void signalHandler(int sign);
};

