#include "Client.hpp"
#include "server.hpp"

Client::Client()
{
}

Client::Client(int fd, Sever &server) : server(server)
{
    
}

Client::~Client()
{
}