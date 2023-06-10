#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <iostream>
#include "server.hpp"



class Server;

class Client
{
    private:
        int fd;
    public:
        Client();
        Client(int fd, Server &server);
        ~Client();
        
};



#endif