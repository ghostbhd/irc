#ifndef CLIENT_HPP
#define CLIENT_HPP
#include <iostream>
#include "Server.hpp"



class Server;

class Client
{
    private:
        int fd;
    public:
        Client();
        ~Client();
        
};



#endif