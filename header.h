#ifndef HEADER_HPP
#define HEADER_HPP

#include <iostream> //for cout
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> //for sockaddr_in
#include <unistd.h> //for read
#include <fcntl.h> //for non_blocking sockets
#include <sys/poll.h> //for multiplexing
#include <cstdlib> //for exit() and EXIT_FAILURE
#include <csignal> //for Cntrl+c / +z
#include <exception> //for exception
#include <cstring> //for memset
#include <vector> //for poll vector

#endif