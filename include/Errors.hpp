#pragma once

// General Errors
#define ERR_NOORIGIN          409 // "409 :No origin specified"
#define ERR_NEEDMOREPARAMS    461 // "461 <command> :Not enough parameters"
#define ERR_ALREADYREGISTERED 462 // "462 :You may not reregister"
#define ERR_PASSWDMISMATCH    464 // "464 :Password incorrect"

// Nickname Errors
#define ERR_NONICKNAMEGIVEN   431 // "431 :No nickname given"
#define ERR_ERRONEUSNICKNAME  432 // "432 <nickname> :Erroneous nickname"
#define ERR_NICKNAMEINUSE     433 // "433 <nickname> :Nickname is already in use"

// User Errors
#define ERR_NOTREGISTERED     451 // "451 <command> :You have not registered"
#define ERR_ALREADYREGISTRED  462 // "462 :You may not reregister"

// Channel Errors
#define ERR_NOSUCHCHANNEL     403 // "403 <channel> :No such channel"
#define ERR_USERONCHANNEL     443 // "443 <user> <channel> :is already on channel"
#define ERR_NOTONCHANNEL      442 // "442 <channel> :You're not on that channel"
#define ERR_CHANNELISFULL     471 // "471 <channel> :Cannot join channel (+l)"
#define ERR_INVITEONLYCHAN    473 // "473 <channel> :Cannot join channel (+i)"
#define ERR_BADCHANNELKEY     475 // "475 <channel> :Cannot join channel (+k)"
#define ERR_UNKNOWNMODE       472 // "472 <char> :Is unknown mode char"
#define ERR_CHANOPRIVSNEEDED  482 // "482 <channel> :You're not channel operator"

// Messaging Errors
#define ERR_NOSUCHNICK        401 // "401 <nickname> :No such nick/channel"
#define ERR_NORECIPIENT       411 // "411 :No recipient given (<command>)"
#define ERR_NOTEXTTOSEND      412 // "412 :No text to send"
#define ERR_CANNOTSENDTOCHAN  404 // "404 <channel> :Cannot send to channel"
#define ERR_TOOMANYTARGETS    407 // "407 <target> :Duplicate recipients. No message delivered"
#define ERR_NOTOPLEVEL        413 // "413 <mask> :No toplevel domain specified"
#define ERR_WILDTOPLEVEL      414 // "414 <mask> :Wildcard in toplevel domain"

// Operator and Admin Errors
#define ERR_NOPRIVILEGES      481 // "481 :Permission Denied- You're not an IRC operator"
#define ERR_CHANOPRIVSNEEDED  482 // "482 <channel> :You're not channel operator"
#define ERR_CANTKILLSERVER    483 // "483 :You can't kill a server!"

// Invite and Kick Errors
#define ERR_USERNOTINCHANNEL  441 // "441 <user> <channel> :They aren't on that channel"
#define ERR_NOTONCHANNEL      442 // "442 <channel> :You're not on that channel"

// Command Errors
#define ERR_UNKNOWNCOMMAND    421 // "421 <command> :Unknown command"
#define ERR_NOTIMPLEMENTED    500 // "500 :Command not implemented"

// Connection and Registration Errors
#define ERR_ALREADYREGISTERED 462 // "462 :You may not reregister"
#define ERR_ERRONEUSNICKNAME  432 // "432 <nickname> :Erroneous nickname"
#define ERR_NICKNAMEINUSE     433 // "433 <nickname> :Nickname is already in use"
#define ERR_PASSWDMISMATCH    464 // "464 :Password incorrect"

// Quit/Disconnect Errors
#define ERR_NOORIGIN          409 // "409 :No origin specified"

// Input/Output Errors
#define ERR_INPUTTOOLONG      417 // "417 :Input line was too long"
