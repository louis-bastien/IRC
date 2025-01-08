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
#include "Server.hpp"
#include "Logger.hpp"

class Channel;
class Server;
class User
{
    private:
        std::string username;
        std::string nickname;
        std::string hostname;
        std::string realname;
        int socket_fd;
        bool is_authenticated; //if pass command is received are verified
        bool is_registered; //if nick user and pass commands are verified
        bool is_visible;
        std::vector<std::string> channels;
        Logger& logger;
    
    public:
        User(int socket_fd, Logger& logger);
        ~User();
        User& operator=(const User& other);
        std::string getNickname() const;
        void setNickname(const std::string& nickname);
        std::string getUsername() const;
        std::vector<std::string>& getChannels();
        void setUsername(const std::string& username);
        void setRealname(const std::string& username);
        std::string getHostname() const;
        std::string getRealname() const;
        void setHostname(const std::string& hostname);
        bool isAuthenticated() const;
        bool isRegistered() const;
        void doRegister(Server& server);
        void sendMessage(const std::string message, bool serverPrefix = true);
        void sendErrorMessage(int errorCode, User& user, std::string message);
        void sendMOTD(Server& server);
        void sendINFO(Server& server);
        void changeMode(std::vector<std::string> params);
        void authenticate();
        int getSocketFd();
        void leaveAllChannels(std::map<std::string, Channel>& allChannels);
};
