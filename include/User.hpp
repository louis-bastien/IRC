#pragma once
#include "irc.hpp"
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

/*
-nickname - max 9 chars;


   CHANNELS
-channel is created implicitly when the first client joins it and stops existing when there is no one;
-channels names are strings (beginning with a '&' or '#') - max 200 chars;
-channel name cannot contain spaces (' '), a control G (^G or ASCII 7), or a comma (',');
-there are 2 types of channels that i didnt figure out yet;
-to join/create a channel -> JOIN; 
-if the channel doesn't exist prior to joining, the channel is created and the creating user becomes a
   channel operator;
-if the channel already exists, whether you can join depends on the current modes 
    of the channel(if the channel is invite-only, (+i), you can only join if invited).
-max channels recommended per person - 10;
    
    OPERATORS
-operators can do
    KICK    - Eject a client from the channel
    MODE    - Change the channel's mode
    INVITE  - Invite a client to an invite-only channel (mode +i)
    TOPIC   - Change the channel topic in a mode +t channel


*/

class User
{
    private:
        std::string username;
        std::string nickname;
        int socket_fd; //client's socket
        bool is_authenticated; //is nick and user commands are verified
        bool is_operator;
        std::vector<std::string> channels;
    
    public:
        User(int socket_fd);
        ~User();
        std::string getNickname() const;
        void setNickname(const std::string& nickname);
        std::string getUsername() const;
        void setUsername(const std::string& username);
        bool isAuthenticated() const;
        void authenticate();
        bool isOperator();
        void sendMessage(const std::string& message);
        void sendPrivateMessage(const std::string& target, const std::string& message);
        std::string receiveMessage();
        void joinChannel(const std::string& channel_name);
        void leaveChannel(const std::string& channel_name);
};
