#include "Channel.hpp"


//IN SERVER WE NEED TO CHECK IF THE NAME IS EMPTY AFTER THE CREATION OF THE CHANNEL BECAUSE IF YES THEN SEND MSG TO THE USER
//Maybe throw std::invalid_argument if name of the channel is wrong
Channel::Channel(std::string& name, Logger& logger) : name(name), topic(""), logger(logger), topic_restricted(false), passPotected(false)
{
    if (name.empty() || name.length() > 200 || 
        (name[0] != '#' && name[0] != '&') ||
        name.find(' ') != std::string::npos || 
        name.find('\a') != std::string::npos || 
        name.find(',') != std::string::npos)
    {
        logger.log(ERROR, "Invalid channel name: " + name);
        this->name = ""; 
        return;
    }
    logger.log(INFO, "Channel created: " + name);
}

Channel::~Channel() 
{
    logger.log(INFO, "Channel destroyed: " + name);
}

//Also add channel to the user container "std::vector<std::string> channels;""
void Channel::addUser(User& user, const std::string& password = "") 
{
    if(passPotected && password != pass_key)
        throw std::invalid_argument("Wrong password for channel " + name + ": " + password);
        
    members.insert(std::make_pair(user.getSocketFd(), user));
    if (members.size() == 1) 
    {
        operators.insert(std::make_pair(user.getSocketFd(), user));
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

void Channel::setTopic(const std::string& topic, User& user) 
{
    if (topic_restricted && !is_operator(user))
    {
        logger.log(WARNING, user.getNickname() + " tried to set the topic for channel " + name + " without permission.");
        user.sendMessage("482 " + user.getNickname() + " :You're not a channel operator");
        return;
    }
    if (topic.length() > 512) 
    {
        logger.log(WARNING, user.getNickname() + " tried to set an overly long topic for channel " + name);
        user.sendMessage("413 " + user.getNickname() + " :Topic is too long");
        return;
    }
    this->topic = topic;
    for (std::map<int, User>::iterator it = members.begin(); it != members.end(); ++it) 
        it->second.sendMessage(":" + user.getNickname() + " TOPIC " + name + " :" + topic);
    logger.log(INFO, user.getNickname() + " set the topic for channel " + name + " to: " + topic);
}

std::string Channel::getName() const
{
    return (this->name);
}

std::string Channel::getTopic() const
{
    return (this->topic);
}

std::map<int, User> Channel::getMembers() const
{
    return (members);
}

bool Channel::is_operator(User& user)
{
    if (operators.find(user.getSocketFd()) != operators.end())
        return (true);
    return (false);
}

bool Channel::isProtected(void)
{
    return (passPotected);
}
