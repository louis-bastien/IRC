#pragma once

//all codes : https://modern.ircdocs.horse/

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

// Channel Errors
#define ERR_NOSUCHCHANNEL     403 // "403 <channel> :No such channel"
#define ERR_USERONCHANNEL     443 // "443 <user> <channel> :is already on channel"
#define ERR_NOTONCHANNEL      442 // "442 <channel> :You're not on that channel"
#define ERR_KEYSET            467 // "467 <channel> :Channel key already set"
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
#define ERR_INPUTTOOLONG      417 // "417 :Input line was too long"

// Successful Command Replies
#define RPL_WELCOME           001 // "001 <nick> :Welcome to the Internet Relay Network <nick>!<user>@<host>"
#define RPL_YOURHOST          002 // "002 <nick> :Your host is <servername>, running version <ver>"
#define RPL_CREATED           003 // "003 <nick> :This server was created <date>"
#define RPL_MYINFO            004 // "004 <nick> <servername> <version> <usermodes> <chanmodes>"
#define RPL_ISUPPORT          005 // "<nick> <1-13 tokens> :are supported by this server"

#define RPL_UMODEIS           221 // "221 <nick> :<usermodes>"
#define RPL_LUSERCLIENT       251 // "251 <nick> :There are <u> users and <i> invisible on <s> servers"
#define RPL_LUSEROP           252 // "252 <nick> <ops> :operator(s) online"
#define RPL_LUSERUNKNOWN      253 // "253 <nick> <connections> :unknown connection(s)"
#define RPL_LUSERCHANNELS     254 // "254 <nick> <channels> :channels formed"

#define RPL_TOPIC             332 // "332 <channel> :<topic>"
#define RPL_NOTOPIC           331 // "331 <channel> :No topic is set"

#define RPL_INVITING          341 // "341 <nick> <channel> :Invitation successful"
#define RPL_NAMREPLY          353 // "353 <nick> = <channel> :<nicknames>"
#define RPL_ENDOFNAMES        366 // "366 <channel> :End of /NAMES list"

#define RPL_LIST              322 // "322 <channel> <visible> :<topic>"
#define RPL_LISTEND           323 // "323 :End of /LIST"
#define RPL_CHANNELMODEIS     324 // "324 <nick> <channel> <chanmodes>"

#define RPL_WHOREPLY          352 // "352 <channel> <user> <host> <server> <nick> <flags> :<hopcount> <realname>"
#define RPL_ENDOFWHO          315 // "315 <channel> :End of /WHO list"

#define RPL_MOTDSTART         375 // "375 :Start of /MOTD message"
#define RPL_MOTD              372 // "372 :<MOTD line>"
#define RPL_ENDOFMOTD         376 // "376 :End of /MOTD command"

#define RPL_YOUREOPER         381 // "381 :You are now an IRC operator"
#define RPL_TIME              391 // "391 <server> :<time>"

#define RPL_VERSION           351 // "351 <version>.<debuglevel> <server> :<comments>"

// Quit/Disconnect Errors
#define ERR_NOORIGIN          409 // "409 :No origin specified"
#define ERR_NOMOTD            422 // "422 <nick> :MOTD File is missing"

// Input/Output Errors
#define ERR_INPUTTOOLONG      417 // "417 :Input line was too long"