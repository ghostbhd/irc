#include "server.hpp"

Server::Server() {}

Server::Server(int port, std::string password) : _pass(password), _port(port)
{
    if (_port < 0 || _port > 65535)
    {
        std::cerr << "Port number is not valid\n";
        exit(EXIT_FAILURE);
    }

    if (_pass.empty())
    {
        std::cerr << "Password is empty\n";
        exit(EXIT_FAILURE);
    }

    this->_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->_sock_fd < 0)
    {
        std::cerr << "Cannot create socket!\n";
        exit(EXIT_FAILURE);
    }

    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_addr.s_addr = INADDR_ANY;
    _sockaddr.sin_port = htons(this->_port);

    _addlen = sizeof(_sockaddr);

    if (bind(this->_sock_fd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr)) < 0) // struct used to specify the @ assigned to the sock
    {
        std::cerr << "Server cannot bind to the port\n";
        exit(EXIT_FAILURE);
    }
    if (listen(this->_sock_fd, 100)) // marks a socket as passive, holds at most 100 connections
    {
        std::cerr << "Server cannot listen on socket\n";
        exit(EXIT_FAILURE);
    }
}

void Server::start()
{
    while (1)
    {
        int fd_cnx = accept(this->_sock_fd, (struct sockaddr *)&_sockaddr, (socklen_t *)&_addlen);
        if (fd_cnx < 0)
        {
            std::cerr << "Server cannot connect\n";
            exit(EXIT_FAILURE);
        }

        std::cout << "Server launched !\n";

        std::vector<char> buff(1024);
        ssize_t rd = read(fd_cnx, buff.data(), 1024);
        if (!rd)
            break;
        std::cout << buff.data();
        recv(this->_sock_fd, buff.data(), 1024, 0);
    }
    // close(fd_cnx);
    close(this->_sock_fd);
}

Server::~Server() { std::cout << "Server is OFF !\n"; }

int Server::getPort() const { return (this->_port); }

int Server::getSock_fd() const { return (this->_sock_fd); }

std::string Server::getPass() const { return (this->_pass); }

const char *Server::Error_Select::what() const throw() { return ("Error: Cannot SELECT socket!\n"); }

const char *Server::Error_Accept::what() const throw() { return ("Error: Cannot Accept socket!\n"); }