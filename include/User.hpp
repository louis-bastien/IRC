#pragma once
#include "irc.hpp"
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "Channel.hpp"
#include "Logger.hpp"


class Channel;


class User
{
    private:
        std::string username;
        std::string nickname;
        int socket_fd;
        bool is_authenticated; //if pass command is received are verified
        bool is_registered; //if nick user and pass commands are verified
        std::vector<std::string> channels;
        Logger& logger;
    
    public:
        User(int socket_fd, Logger& logger);
        ~User();
        User& operator=(const User& other);
        std::string getNickname() const;
        void setNickname(const std::string& nickname);
        std::string getUsername() const;
        void setUsername(const std::string& username);
        bool isAuthenticated() const;
        bool isRegistered() const;
        void sendMessage(const std::string& message);
        void leaveChannel(Channel& channel, std::string& reason);
        bool canRegister() const;
        void authenticate();
        void doRegister();
        int getSocketFd();
        void leaveAllChannels(std::map<std::string, Channel>& allChannels);
};
