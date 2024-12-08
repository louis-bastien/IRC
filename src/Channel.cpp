#include "Channel.hpp"
#include "User.hpp"

Channel::Channel(std::string& name) : name(name), topic("") 
{
    logger.log(INFO, "Channel created: " + name);
}

Channel::~Channel() 
{
    logger.log(INFO, "Channel destroyed: " + name);
}

void Channel::addUser(User& user) 
{
    members[user.getSocketFd()] = user;

    // if its the first user, they become the operator ??
    if (members.size() == 1) 
    {
        operators[user.getSocketFd()] = user;
        logger.log(INFO, user.getNickname() + " is the operator of the channel " + name);
    }
    logger.log(INFO, user.getNickname() + " joined channel " + name);
}

void Channel::removeUser(User& user) 
{
    members.erase(user.getSocketFd());
    operators.erase(user.getSocketFd());

    logger.log(INFO, user.getNickname() + " left channel " + name);
}