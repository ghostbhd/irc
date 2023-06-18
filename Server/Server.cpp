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

    // memset(&_sockaddr, 0, sizeof(_sockaddr));

    this->_sock_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET=> IPV4, SOCK_STREAM=>TCP
    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_addr.s_addr = INADDR_ANY;
    _sockaddr.sin_port = htons(this->_port);

    if (this->_sock_fd < 0)
    {
        std::cerr << "Cannot create socket!\n";
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // if (setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    // {
    //     std::cerr << "Cannot reuse the address\n";
    //     exit(EXIT_FAILURE);
    // }
    int binding = bind(this->_sock_fd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr));
    if (binding < 0) // struct used to specify the @ assigned to the sock
    {
        std::cerr << "Server cannot bind to the address/port \n";
        exit(EXIT_FAILURE);
    }
    int listening = listen(this->_sock_fd, 100);
    if (listening < 0) // marks a socket as passive, holds at most 100 connections
    {
        std::cerr << "Server cannot listen on socket\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Server launched !\n";
}

// Main functions ---------------------------------------------------------------------------------------
void Server::start()
{
    pollfd server_poll;
    memset(&server_poll, 0, sizeof(server_poll));

    server_poll.fd = this->_sock_fd;
    server_poll.events = POLLIN;

    this->_poll_vc.push_back(server_poll);
    initErrorMsg();
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
    Client cl(client_fd, _pass);
    _clients.insert(std::make_pair(client_fd, cl));
    std::cout << "Client " << client_fd << " connected!\n";
}

void Server::ClientRecv(int client_fd)
{
    std::vector<char> buffer(5000);
    size_t rd = read(client_fd, buffer.data(), buffer.size());
    if (!rd)
    {
        std::cout << "Client " << client_fd << " disconnected!\n";
        close(client_fd);
    }
    std::string str = deleteNewLine(buffer.data());
    if (str.empty())
        return;
    size_t pos = str.find(":");
    if (pos == std::string::npos)
    {
        // <client> <command> :Unknown command
        sendError(client_fd, ERR_NEEDMOREPARAMS);
        return;
    }
    std::string cmd = str.substr(0, pos);
    if (_clients[client_fd].getAuth() == false)
    {
        if (cmd != "PASS")
            sendError(client_fd, ERR_NOTREGISTERED);
        else
        {
            std::string pass(str.substr(pos + 2));
            if (pass != _pass)
                sendError(client_fd, ERR_PASSWDMISMATCH);
            else
                _clients[client_fd].setAuth(true);
        }
    }
    else
    {
        if (_clients[client_fd].getNickname().empty() || _clients[client_fd].getUsername().empty())
        {
            if (cmd == "NICK")
            {
                std::string nick(str.substr(pos + 2));
                if (nick.empty())
                    sendError(client_fd, ERR_NEEDMOREPARAMS);
                else
                {
                    if (_clients[client_fd].getNickname().empty())
                        _clients[client_fd].setNickname(nick);
                    else
                        sendError(client_fd, ERR_NICKNAMEINUSE);
                }
            }
            else if (cmd == "USER")
            {
                std::string user(str.substr(pos + 2));
                if (user.empty())
                    sendError(client_fd, ERR_NEEDMOREPARAMS);
                else
                {
                    if (_clients[client_fd].getUsername().empty())
                        _clients[client_fd].setUsername(user);
                    else
                        sendError(client_fd, ERR_ALREADYREGISTERED);
                }
            }
            else
                sendError(client_fd, ERR_NOTREGISTERED);
        }
        else
        {
            // Ather commands
        }
    }
}

// Utils ------------------------------------------------------------------------------------------------
std::string Server::deleteNewLine(char* str)
{
    std::string s(str);

    std::string::size_type pos = 0;
    while ((pos = s.find('\n', pos)) != std::string::npos)
    {
        s.erase(pos, 1);
    }

    pos = 0;
    while ((pos = s.find('\r', pos)) != std::string::npos)
    {
        s.erase(pos, 1);
    }

    return s;
}


// Error handling ---------------------------------------------------------------------------------------
void Server::initErrorMsg()
{

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

    _errorMsg.insert(std::make_pair(433, " :Nickname is already in use\n"));
    _errorMsg.insert(std::make_pair(443, " :is already on channel\n"));
    _errorMsg.insert(std::make_pair(451, " :You have not registered\n"));
    _errorMsg.insert(std::make_pair(461, " :Not enough parameters\n"));
    _errorMsg.insert(std::make_pair(464, " :Password incorrect\n"));
    _errorMsg.insert(std::make_pair(501, " :Unknown MODE flag\n"));
    _errorMsg.insert(std::make_pair(462, " :You may not reregister\n"));
}

void Server::sendError(int client_fd, int error_code) // need to add channel name to error msg
{
    std::string error;
    std::string fd_string;
    std::stringstream ss;
    ss << client_fd;
    ss >> fd_string;
    if (error_code == 433)
        error = fd_string + " " + _clients[client_fd].getNickname() + _errorMsg[error_code];
    // else if (error_code == 443)
    //     error = fd_string + " " + _clients[client_fd].getNickname() + _errorMsg[error_code];
    else if (error_code == 451)
        error = fd_string + _errorMsg[error_code];
    else if (error_code == 461)
        error = fd_string + _errorMsg[error_code];
    else if (error_code == 464)
        error = fd_string + _errorMsg[error_code];
    else if (error_code == 501)
        error = fd_string + _errorMsg[error_code];
    else if (error_code == 462)
        error = fd_string + _errorMsg[error_code];

    send(client_fd, error.c_str(), error.size(), 0);
}

// Destructor -------------------------------------------------------------------------------------------
Server::~Server() { std::cout << "Server is OFF !\n"; }

// Getters ----------------------------------------------------------------------------------------------
int Server::getPort() const { return (this->_port); }         // _port
int Server::getSock_fd() const { return (this->_sock_fd); }   // _sock_fd
std::string Server::getPass() const { return (this->_pass); } // _pass

// Exceptions --------------------------------------------------------------------------------------------
const char *Server::Error_Select::what() const throw() { return ("Error: Cannot SELECT socket!\n"); }

const char *Server::Error_Accept::what() const throw() { return ("Error: Cannot Accept socket!\n"); }