# pragma once
#include "irc.hpp"
#include "User.hpp"
#include "Server.hpp"
#include "Logger.hpp"

//he

class Channel
{
    private:
        std::string name;
        std::map<int, User> operators;
        std::map<int, User> members;
        std::string topic;

    public:
        Channel(std::string& name);
        ~Channel();
        void addUser(User& user);
        void removeUser(User&user);
        void setTopic(const std::string& topic);
        std::string& getTopic() const;
        bool is_operator(User& user);

};