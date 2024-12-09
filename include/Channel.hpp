# pragma once
#include "irc.hpp"
#include "User.hpp"
#include "Server.hpp"
#include "Logger.hpp"
#include <algorithm> 

class User;

class Channel
{
    private:
        std::string name;
        std::map<int, User> operators;
        std::map<int, User> members;
        std::string topic;
        Logger& logger;
        bool    topic_restricted;

    public:
        Channel(std::string& name, Logger& logger);
        ~Channel();
        void addUser(User& user);
        void removeUser(User&user);
        void setTopic(const std::string& topic, User& user);
        std::string getName() const;
        std::string getTopic() const;
        bool is_operator(User& user);
};