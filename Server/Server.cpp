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

    memset(&_sockaddr, 0, sizeof(_sockaddr)); //setting the memory to 0;

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
    if (setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        std::cerr << "Cannot reuse the address\n";
        exit(EXIT_FAILURE);
    }
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
    size_t ReadingFromC = read(client_fd, buffer.data(), buffer.size());
    if (!ReadingFromC)
    {
        std::cout << "Client " << client_fd << " disconnected!\n";
        _clients.erase(client_fd);

        close(client_fd);
    }

    // Getting the line from the buffer and deleting the \n and \r
    std::string CleanLine = deleteNewLine(buffer.data());
    if (CleanLine.empty())
        return;

    // Getting the position of the first space
    size_t pos = CleanLine.find(" ");
    // check if the line is valid
    if (pos == std::string::npos || pos == 0 || CleanLine[pos + 1] == '\0')
    {
        sendError(client_fd, ERR_NEEDMOREPARAMS, CleanLine);
        return;
    }

    // Parameter before ":" ex: NICK, USER, PASS, etc...
    std::string cmd = CleanLine.substr(0, pos);

    // Pass if passwd is correct then auth = true
    if (_clients[client_fd].getAuth() == false)
    {
        if (cmd != "PASS" && cmd != "pass")
            sendError(client_fd, ERR_NOTREGISTERED, CleanLine);
        else
        {
            std::string pass(CleanLine.substr(pos + 1));
            if (pass != _pass)
                sendError(client_fd, ERR_PASSWDMISMATCH, CleanLine);
            else
                _clients[client_fd].setAuth(true);
        }
    }
    // NICK & USER *********** and other commands if the NICK and USER are set
    else
    {
        if (_clients[client_fd].getNickname().empty() || _clients[client_fd].getUsername().empty())
        {
            if (cmd == "NICK" || cmd == "nick")
            {
                std::vector<std::string> nick = splitWithSpaces(CleanLine.substr(pos + 1));
                if (nick.size() != 1)
                    sendError(client_fd, ERR_NEEDMOREPARAMS, CleanLine);
                else
                {
                    if (_clients[client_fd].getNickname().empty())
                        _clients[client_fd].setNickname(nick[0]);
                    else
                        sendError(client_fd, ERR_NICKNAMEINUSE, CleanLine);
                }
            }
            else if (cmd == "USER" || cmd == "user")
            {
                std::vector<std::string> user = splitWithSpaces(CleanLine.substr(pos + 1));

                if (user.size() != 1)
                    sendError(client_fd, ERR_NEEDMOREPARAMS, CleanLine);
                else
                {
                    if (_clients[client_fd].getUsername().empty())
                        _clients[client_fd].setUsername(user[0]);
                    else
                        sendError(client_fd, ERR_ALREADYREGISTERED, CleanLine);
                }
            }
            else
                sendError(client_fd, ERR_NOTREGISTERED, CleanLine);
            if (!_clients[client_fd].getUsername().empty() && !_clients[client_fd].getNickname().empty())
                sendWelcomeRpl(client_fd, WELCOMINGCODE);
        }
        else
        {
            // All Other Commands Here : OPER / JOIN / PRIVMSG / ...
            mainCommands(client_fd, &CleanLine[pos + 2], cmd);
        }
    }
}

// Utils ------------------------------------------------------------------------------------------------
std::string Server::deleteNewLine(char *str)
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

std::vector<std::string> Server::splitWithSpaces(std::string str)
{
    std::vector<std::string> result;
    std::string tmp;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == ' ')
        {
            result.push_back(tmp);
            tmp.clear();
        }
        else
            tmp += str[i];
    }
    result.push_back(tmp);
    return result;
}

int Server::findClientFdByNick(std::string nick)
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (it->second.getNickname() == nick)
            return it->first;
    }
    return -1;
}

int Server::findClientFdByUser(std::string user)
{
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (it->second.getUsername() == user)
            return it->first;
    }
    return -1;
}

// Send Replay ------------------------------------------------------------------------------------------
void Server::sendWelcomeRpl(int client_fd, int code)
{
    std::stringstream cl_fd;
    std::string fd_str;
    cl_fd << client_fd;
    cl_fd >> fd_str;
    std::string message;

    message = fd_str + " :Welcome to the IRC Network, " + this->_clients[client_fd].getNickname() + "[!" + _clients[client_fd].getUsername() + "@" + _clients[client_fd].getHostname() + "]\n";
    if (code == 001)
        send(client_fd, message.c_str(), message.size(), 0);
}

// Error handling ---------------------------------------------------------------------------------------
void Server::initErrorMsg()
{
    _errorMsg.insert(std::make_pair(433, " :Nickname is already in use\n"));
    _errorMsg.insert(std::make_pair(443, " :is already on channel\n"));
    _errorMsg.insert(std::make_pair(451, " :You have not registered\n"));
    _errorMsg.insert(std::make_pair(461, " :Not enough parameters\n"));
    _errorMsg.insert(std::make_pair(464, " :Password incorrect\n"));
    _errorMsg.insert(std::make_pair(501, " :Unknown MODE flag\n"));
    _errorMsg.insert(std::make_pair(462, " :You may not reregister\n"));
}

void Server::sendError(int client_fd, int error_code, std::string command) // need to add channel name to error msg
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
        error = fd_string + " " + command + _errorMsg[error_code];
    else if (error_code == 464)
        error = fd_string + _errorMsg[error_code];
    else if (error_code == 501)
        error = fd_string + _errorMsg[error_code];
    else if (error_code == 462)
        error = fd_string + _errorMsg[error_code];

    send(client_fd, error.c_str(), error.size(), 0);
}

// Destructor -------------------------------------------------------------------------------------------
Server::~Server()
{
    // close all fd
    std::map<int, Client>::iterator it = _clients.begin();
    while (it != _clients.end())
    {
        close(it->first);
        it++;
    }
    close(_sock_fd);
    std::cout << "Server is OFF !\n";
}

// Commands ---------------------------------------------------------------------------------------------
void Server::mainCommands(int client_fd, std::string cleanLine, std::string cmd)
{
    if (cmd == "OPER" || cmd == "oper")
        operCmd(client_fd, cleanLine);
    else
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
}

void Server::operCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> oper = splitWithSpaces(cleanLine);
    if (oper.size() != 2)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
        if (oper[0] == _adminName && oper[1] == _adminPass)
        {
            _clients[client_fd].setOperator(true);
            // send oper reply
        }
        else
            sendError(client_fd, ERR_PASSWDMISMATCH, cleanLine);
    }
}

// Getters ----------------------------------------------------------------------------------------------
int Server::getPort() const { return (this->_port); }         // _port
int Server::getSock_fd() const { return (this->_sock_fd); }   // _sock_fd
std::string Server::getPass() const { return (this->_pass); } // _pass
