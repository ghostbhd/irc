#ifndef HEADER_HPP
#define HEADER_HPP

#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/poll.h>
#include <cstdlib>
#include <string>
#include <csignal>
#include <exception>
#include <cstring>
#include <vector>
#include <map>
#include <sstream>


enum ERROR_CODE
{
    ERR_NICKNAMEINUSE = 433,
    ERR_USERONCHANNEL = 443,
    ERR_NOTREGISTERED = 451,
    ERR_NEEDMOREPARAMS = 461,
    ERR_PASSWDMISMATCH = 464,
    ERR_UMODEUNKNOWNFLAG = 501,
    ERR_ALREADYREGISTERED = 462,
    ERR_NOSUCHNICK = 401,
    ERR_TOOMANYTARGETS = 407,
    ERR_NOSUCHCHANNEL = 403,
    ERR_BADCHANNELKEY = 475,
    ERR_TOOMANYCHANNELS = 405,
    ERR_CHANOPRIVSNEEDED = 482,
    ERR_NOTONCHANNEL = 442,
    ERR_UNKNOWNMODE = 472,
    ERR_UNKNOWNCOMMAND = 421,
    ERR_INVITEONLYCHAN = 473,
    ERR_NORECIPIENT  = 411,
    ERR_CANNOTSENDTOCHAN = 404,
    ERR_CHANNELISFULL = 471,
    ERR_NOPRIVILEGES = 481,
};

#endif