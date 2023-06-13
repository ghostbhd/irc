#include "Server.hpp"

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

    memset(&_sockaddr, 0, sizeof (_sockaddr));

    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_addr.s_addr = INADDR_ANY;
    _sockaddr.sin_port = htons(this->_port);

    this->_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (this->_sock_fd < 0)
    {
        std::cerr << "Cannot create socket!\n";
        exit(EXIT_FAILURE);
    }

    //int opt = 1;
    // if (setsockopt(this->_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) //to reuse port & bind into the @ 
    // {
    //     std::cerr << "Server cannot reuse socket add\n";
    //     exit(EXIT_FAILURE);
    // }
    // if (setsockopt(this->_sock_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) //to reuse port & bind into the @ 
    // {
    //     std::cerr << "Server cannot reuse socket port\n";
    //     exit(EXIT_FAILURE);
    // }
    // if (fcntl(this->_sock_fd, F_SETFL, O_NONBLOCK) < 0) //will not block the program's execution, allowing it to continue running even if there is no data to read or if writing would block.
    // {
    //     std::cerr << "Blocking mode in socket\n";
    //     exit(EXIT_FAILURE);
    // }
    int binding = bind(this->_sock_fd, (struct sockaddr*) &_sockaddr, sizeof(_sockaddr));
    if (binding < 0) //struct used to specify the @ assigned to the sock
    {
        std::cerr << "Server cannot bind to the port\n";
        exit (EXIT_FAILURE);
    }
    int listening = listen(this->_sock_fd, 100);
    if (listening < 0) //marks a socket as passive, holds at most 100 connections
    {
        std::cerr << "Server cannot listen on socket\n";
        exit (EXIT_FAILURE);
    }
}


void Server::start()
{
    int len = sizeof(sockaddr);
    int connection = accept(this->_sock_fd, (struct sockaddr*)&_sockaddr, (socklen_t*)&len);
    if (connection < 0)
    {
        std::cerr << "Server cannot connect\n";
        exit (EXIT_FAILURE);
    }
    std::cout << "Server launched !\n";
    while (1)
    {
        std::vector<char> buff(1024);
        size_t rd = read(connection, buff.data(), 1024);
        if (!rd)
            break;
        std::cout << buff.data();
        recv(this->_sock_fd, buff.data(), 1024, 0);
    }
    close(connection);
    close(this->_sock_fd);
}

Server::~Server() { std::cout << "Server is OFF !\n"; }

int Server::getPort() const { return (this->_port); }

int Server::getSock_fd() const { return (this->_sock_fd); }

std::string Server::getPass() const { return (this->_pass); }

const char *Server::Error_Select::what() const throw() { return ("Error: Cannot SELECT socket!\n"); }

const char *Server::Error_Accept::what() const throw() { return ("Error: Cannot Accept socket!\n"); }