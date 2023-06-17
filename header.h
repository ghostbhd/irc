#ifndef HEADER_HPP
#define HEADER_HPP

#include <iostream> //for cout
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> //for sockaddr_in
#include <unistd.h> //for read
#include <fcntl.h> //for non_blocking sockets
#include <sys/poll.h> //for multiplexing
#include <cstdlib> //for exit() and EXIT_FAILURE
#include <csignal> //for Cntrl+c 
#include <exception> //for exception
#include <cstring> //for memset
#include <vector> //for poll vector
#include <map> //for map


/*
        ERR_NICKNAMEINUSE (433)
        "<client> <nick> :Nickname is already in use"

        ERR_USERONCHANNEL (443)
        "<client> <nick> <channel> :is already on channel"

        ERR_NOTREGISTERED (451)
        "<client> :You have not registered"

        ERR_NEEDMOREPARAMS (461)
        "<client> <command> :Not enough parameters"

        ERR_PASSWDMISMATCH (464)
        "<client> :Password incorrect"

        ERR_UMODEUNKNOWNFLAG (501)
        "<client> :Unknown MODE flag"

        ERR_ALREADYREGISTERED (462) 
        "<client> :You may not reregister"
    */

enum ERROR_CODE
{
    ERR_NICKNAMEINUSE = 433,
    ERR_USERONCHANNEL = 443,
    ERR_NOTREGISTERED = 451,
    ERR_NEEDMOREPARAMS = 461,
    ERR_PASSWDMISMATCH = 464,
    ERR_UMODEUNKNOWNFLAG = 501,
    ERR_ALREADYREGISTERED = 462
};

#endif