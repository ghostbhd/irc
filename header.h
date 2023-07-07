#ifndef HEADER_HPP
#define HEADER_HPP

#include <iostream> //for cout
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> //for sockaddr_in
#include <unistd.h> //for read
#include <fcntl.h> //for non_blocking sockets
#include <sys/poll.h> //for multiplexing
#include <cstdlib> //for exit() and EXIT_FAILURE
#include <string> //for string
#include <csignal> //for Cntrl+c 
#include <exception> //for exception
#include <cstring> //for memset
#include <vector> //for poll vector
#include <map> //for map
#include <sstream> // for stringstream


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
};

#endif