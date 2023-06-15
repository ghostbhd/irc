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

    memset(&_sockaddr, 0, sizeof(_sockaddr));

    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_addr.s_addr = INADDR_ANY;
    _sockaddr.sin_port = htons(this->_port);

    this->_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (this->_sock_fd < 0)
    {
        std::cerr << "Cannot create socket!\n";
        exit(EXIT_FAILURE);
    }

    int binding = bind(this->_sock_fd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr));
    if (binding < 0) // struct used to specify the @ assigned to the sock
    {
        std::cerr << "Server cannot bind to the port\n";
        exit(EXIT_FAILURE);
    }
    int listening = listen(this->_sock_fd, 100);
    if (listening < 0) // marks a socket as passive, holds at most 100 connections
    {
        std::cerr << "Server cannot listen on socket\n";
        exit(EXIT_FAILURE);
    }
}

void Server::start()
{
    // Declaring poll struct FOR SERVER SIDE
    pollfd server_poll;
    memset(&server_poll, 0, sizeof(server_poll));
    // FILLING THE STRUCT FOR THE SERVER SIDE
    server_poll.fd = this->_sock_fd;
    server_poll.events = POLLIN;

    // setting the memory to zero

    // declaring socket client

    // FILL A VECTOR OF SOCKETS = SERVERS, stock them in vec
    this->_poll_vc.push_back(server_poll);

    while (1)
    {
        if (poll(_poll_vc.data(), _poll_vc.size(), 0) < 0)
        {
            std::cerr << "Cannot connect with multiple clients\n";
            exit(EXIT_FAILURE);
        }
        for (unsigned long i = 0; i < _poll_vc.size(); i++)
        {
            pollfd &current = _poll_vc[i];
            if (current.revents & POLLIN)
            {
                if (current.fd == _sock_fd)
                    newClient();
                else
                    ClientRecv(current.fd);
            }
        }
    }
}

void Server::newClient()
{
    struct pollfd client_poll;
    struct sockaddr_in addr_client;
    socklen_t len = sizeof(addr_client);

    int client_fd = accept(_sock_fd, (struct sockaddr *)&addr_client, &len);

    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    if (client_fd < 0)
    {
        std::cerr << "Cannot accept client\n";
        exit(EXIT_FAILURE);
    }

    client_poll.fd = client_fd;
    client_poll.events = POLLIN;
    _poll_vc.push_back(client_poll);
    send(client_fd, "Enter Password: ", 17, 0);
}

void Server::ClientRecv(int client_fd)
{
    std::vector<char> buffer(1024);
    size_t rd = read(client_fd, buffer.data(), 1024);
    if (!rd)
        close(client_fd);
    std::cout << buffer.data();
    recv(client_fd, buffer.data(), 1024, 0);
}

Server::~Server() { std::cout << "Server is OFF !\n"; }

int Server::getPort() const { return (this->_port); }

int Server::getSock_fd() const { return (this->_sock_fd); }

std::string Server::getPass() const { return (this->_pass); }

const char *Server::Error_Select::what() const throw() { return ("Error: Cannot SELECT socket!\n"); }

const char *Server::Error_Accept::what() const throw() { return ("Error: Cannot Accept socket!\n"); }