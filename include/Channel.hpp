# pragma once
#include "irc.hpp"
#include "User.hpp"
#include "Server.hpp"
#include "Logger.hpp"
#include <algorithm> 

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
        std::map<int, User> invited;
        std::string topic;
        Logger& logger;
        bool    topic_restricted;
        bool    invite_only;
        bool    is_protected;
        std::string password;

    public:
        Channel(std::string& name, Logger& logger);
        ~Channel();
        void addUser(User& user, std::string password="");
        void removeUser(User& user, std::string reason);
        void setTopic(User& user, const std::string& topic);
        std::string getName() const;
        std::string getTopic() const;
        std::map<int, User> getMembers() const;
        bool is_operator(User& user); // create a function that will handle cases if is not operator
        bool is_member(User& user);
        void kickUser(User& operator_user, std::string& target_user, std::string& reason); // is operator tries to kick then error
        void inviteUser(User& operator_user, std::string& tar_user, std::map<int, User>& Users); 
        void changeMode(User& operator_user, std::vector<std::string>);
        bool isProtected(void);
        void broadcast(std::string msg);
};