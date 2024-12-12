# pragma once
#include "irc.hpp"
#include "User.hpp"
#include "Server.hpp"
#include "Logger.hpp"
#include <algorithm> 

// invite create a list and check in adduser
// broadcast fucn; implement throw; parse mode fucntions; 
//mode(user& user, std::vector<std::string> params) 
//kick (user_op, std::string(user_trget), std::string (reason = "Goodbye"))
//(part) leave channel ->reason (remove user)
// broadcast everything(change the mode etc)
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
        bool    invite_only;
        bool    is_protected;
        std::string password;

    public:
        Channel(std::string& name, Logger& logger);
        ~Channel();
        void addUser(User& user, const std::string& password = "");
        void removeUser(User&user); //add the reason as a second argument to broadcast to channel
        void setTopic(const std::string& topic, User& user);
        std::string getName() const;
        std::string getTopic() const;
        std::map<int, User> getMembers() const;
        bool is_operator(User& user);
        void kickUser(User& operator_user, User& target_user, const std::string& reason);
        void inviteUser(User& operator_user, User& target_user);
        void changeMode(User& operator_user, std::vector<std::string>);
        bool isProtected(void)

};