#include "server.hpp"

Server::Server()
{
}

Server::Server(int port, std::string password)
{
    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(this->port);

    std::memset(&sockaddr, 0, sizeof sockaddr);

    this->server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->server_fd < 0)
    {
        std::cerr << "Socket_fd Failed !\n";
        exit(EXIT_FAILURE);
    }


}


Server::~Server()
{
}