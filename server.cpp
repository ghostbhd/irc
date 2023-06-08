#include "server.hpp"

Server::Server()
{
}

Server::Server(int port, std::string password)
{
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET; //address family ==> IPv4
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(this->port); //convert a number to network byte order

    std::memset(&sockaddr, 0, sizeof sockaddr);

    this->server_fd = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM => create a TCP socket
    if (this->server_fd < 0)
    {
        std::cerr << "Cannot create socket!\n";
        exit(EXIT_FAILURE);
    }
    

}


Server::~Server()
{
}

int const &Server::getPort()const
{
    return (this->port);
}

int const &Server::getServer_fd()const
{
    return (this->server_fd);
}

std::string const &Server::getPass()const
{
    return (this->pass);
}