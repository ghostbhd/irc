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

    _adminName = "admin";
    _adminPass = "admin";

    memset(&_sockaddr, 0, sizeof(_sockaddr)); // setting the memory to 0;

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
                sendWelcomeRpl(client_fd,"", WELCOMINGCODE,"");
        }
        else
        {
            // All Other Commands Here : OPER / JOIN / PRIVMSG / ...
            mainCommands(client_fd, &CleanLine[pos + 1], cmd);
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

bool Server::isChannelExist(std::string name)
{
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if (it->first == name)
            return true;
    }
    return false;
}
// Client ****************************************
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

// Channel ***************************************
std::string Server::findChannelByFd(int fd)
{
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if (it->second.isChanMember(_clients[fd].getNickname()))
            return it->first;
    }
    return "";
}

bool Server::isChanNameValid(std::string name)
{
    if (name[0] != '#' || name == "#")
        return false;
    return true;
}
// Send Replay ------------------------------------------------------------------------------------------
void Server::sendWelcomeRpl(int client_fd, std::string nick, int code, std::string Channel)
{
    std::stringstream cl_fd;
    std::string fd_str;
    cl_fd << client_fd;
    cl_fd >> fd_str;

    std::string message = fd_str + " :Welcome to the IRC Network, " + this->_clients[client_fd].getNickname() + "[!" + _clients[client_fd].getUsername() + "@" + _clients[client_fd].getHostname() + "]\n";
    std::string msg = fd_str + " :You are now an IRC operator\n";
    std::string inv = fd_str + " has been invited " + nick + " to " + Channel +"\n";
    std::string tpc = fd_str + " this channel " + nick + " is using the current topic " + Channel + "\n";

    if (code == 001)
        send(client_fd, message.c_str(), message.size(), 0);
    else if (code == 381)
        send(client_fd, msg.c_str(), msg.size(), 0);
    else if (code == 341)
        send(client_fd, inv.c_str(), inv.size(), 0);
    else if (code == 332)
        send(client_fd, tpc.c_str(), tpc.size(), 0);
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
    _errorMsg.insert(std::make_pair(401, " :No such nick/channel\n"));
    _errorMsg.insert(std::make_pair(401, " :Too many targets\n"));
    _errorMsg.insert(std::make_pair(403, " :No such channel\n"));
    _errorMsg.insert(std::make_pair(475, " :Cannot join channel (+k)\n"));
    _errorMsg.insert(std::make_pair(405, " :You are already registred\n"));
    _errorMsg.insert(std::make_pair(482, " :You're not channel operator\n"));
    _errorMsg.insert(std::make_pair(442, " :You're not on that channel\n"));
    _errorMsg.insert(std::make_pair(472, " :is unknown mode char to me\n"));
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
    else if (error_code == 443)
        error = fd_string + " " + _clients[client_fd].getNickname() + " " + command + _errorMsg[error_code];
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
    else if (error_code == 401)
        error = fd_string + " " + _clients[client_fd].getNickname() + _errorMsg[error_code];
    else if (error_code == 407)
        error = fd_string + " " + _errorMsg[error_code];
    else if (error_code == 403)
        error = fd_string + " " + command + _errorMsg[error_code];
    else if (error_code == 475)
        error = fd_string + " " + command + _errorMsg[error_code];
    else if (error_code == 405)
        error = fd_string + " " + command + _errorMsg[error_code];
    else if (error_code == 482)
        error = fd_string + " " + command + _errorMsg[error_code];
    else if (error_code == 442)
        error = fd_string + " " + command + _errorMsg[error_code];
    else if (error_code == 472)
        error = fd_string + " " + command + _errorMsg[error_code];

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
    if (cmd == "OPER")
        operCmd(client_fd, cleanLine);
    else if (cmd == "PRIVMSG")
        privmsg(client_fd, cleanLine);
    else if (cmd == "JOIN")
        joinCmd(client_fd, cleanLine);
    else if (cmd == "INVITE")
        inviteCmd(client_fd, cleanLine);
    else if (cmd == "KICK")
        KickCmd(client_fd, cleanLine);
    else if (cmd == "TOPIC")
        topicCmd(client_fd, cleanLine);
    // else if (cmd == "MODE")
    //     modeCmd(client_fd, cleanLine);
    else
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
}

// OPER >>>>>>>>>>>>>>>>>>>>>>>>
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
            // send oper RPL
            sendWelcomeRpl(client_fd, "",RPL_AWAY,"");
        }
        else
            sendError(client_fd, ERR_PASSWDMISMATCH, cleanLine);
    }
}

// PRIVMSG >>>>>>>>>>>>>>>>>>>>
void Server::privmsg(int client_fd, std::string cleanLine)
{
    std::vector<std::string> msg = splitWithSpaces(cleanLine);
    if (msg.size() < 2)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
        int fd = findClientFdByNick(msg[0]);
        if (fd == -1)
            sendError(client_fd, ERR_NOSUCHNICK, cleanLine); // error nick not found
        else
        {
            if (msg[1][0] != ':')
                sendError(client_fd, ERR_TOOMANYTARGETS, cleanLine);
            else
            {
                std::string msg = ":" + _clients[fd].getNickname() + " PRIVMSG " + _clients[client_fd].getNickname() + " " + cleanLine.substr(cleanLine.find(":")) + "\n";
                send(fd, msg.c_str(), msg.size(), 0);
            }
        }
    }
}

// JOIN >>>>>>>>>>>>>>>>>>>>>>>>
void Server::joinCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> join = splitWithSpaces(cleanLine); // split line with spaces

    std::string nick = _clients[client_fd].getNickname(); // get client nickname
    if (join.size() < 1)                                  // no channel name
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
        if (!isChanNameValid(join[0])) // channel name must be valid
            sendError(client_fd, ERR_NOSUCHCHANNEL, join[0]);
        else
        {
            std::string key = "";
            if (join.size() > 1)
                key = cleanLine.substr(cleanLine.find(join[1])); // get key
            if (!isChannelExist(join[0]))                        // channel not found create it and add client
            {
                Channel newChannel(join[0], key, nick);                // create new channel (key, name, chanOps)
                _channels.insert(std::make_pair(join[0], newChannel)); // add channel to map
                _channels[join[0]].addClient(nick);                    // add chanops to channel as client
                std::string msg = ":" + nick + " JOIN " + join[0] + "\n";
                send(client_fd, msg.c_str(), msg.size(), 0);
                std::cout << "New channel created : " << join[0] << " by " << nick << std::endl;
            }
            else // channel found add client to it
            {
                if (_channels[join[0]].getKey().empty()) // no key
                {
                    if (_channels[join[0]].isChanMember(nick)) // client already in channel
                        sendError(client_fd, ERR_TOOMANYCHANNELS, join[0]);
                    else // add client to channel (no key)
                    {
                        _channels[join[0]].addClient(nick);
                        std::string msg = ":" + nick + " JOIN " + join[0] + "\n";
                        send(client_fd, msg.c_str(), msg.size(), 0);
                    }
                }
                else // key needed
                {
                    if (_channels[join[0]].getKey() == key) // key match add client
                    {
                        _channels[join[0]].addClient(nick);
                        std::string msg = ":" + nick + " JOIN " + join[0] + "\n";
                        send(client_fd, msg.c_str(), msg.size(), 0);
                    }
                    else
                        sendError(client_fd, ERR_BADCHANNELKEY, join[0]);
                }
            }
        }
    }
}

// INVITE >>>>>>>>>>>>>>>>>>>>>>>>
void Server::inviteCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> inv = splitWithSpaces(cleanLine);

    std::string nick = _clients[client_fd].getNickname(); // get client nickname

    if (inv.size() < 2) // CMD + NICK + CHANNEL
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
        if (!isChanNameValid(inv[1])) // channel name must be valid
            sendError(client_fd, ERR_NOSUCHCHANNEL, inv[1]);
        else
        {
            if (!isChannelExist(inv[1])) // channel not found
                sendError(client_fd, ERR_NOSUCHCHANNEL, inv[1]);
            else
            {
                if (_channels[inv[1]].getInviteOnly() && !_channels[inv[1]].isChanOps(nick))
                    sendError(client_fd, ERR_CHANOPRIVSNEEDED, inv[1]);
                else
                {
                    int cInvited = findClientFdByNick(inv[0]);
                    if (cInvited == -1) // check if the client exists in the server
                        sendError(client_fd, ERR_NOSUCHNICK, inv[0]);
                    else
                    {
                        std::string invitedNick = _clients[cInvited].getNickname();
                        if (!_channels[inv[1]].isChanMember(nick)) // check if the inviter is in the channel
                            sendError(client_fd, ERR_USERONCHANNEL, inv[1]);
                        else
                        {
                            if (_channels[inv[1]].isChanMember(invitedNick)) // check if the invited client is already in the channel
                                sendError(client_fd, ERR_USERONCHANNEL, inv[1]);
                            else
                            {
                                _channels[inv[1]].addClient(invitedNick);
                                std::string msg = inv[0] + " is a member in " + inv[1] + "\n";
                                sendWelcomeRpl(client_fd, inv[0] ,RPL_INVITING, inv[1]);
                                send(cInvited, msg.c_str(), msg.size(), 0);
                                std::cout << _clients[client_fd].getNickname() << " has invited " << inv[0] << " to the channel " << inv[1] << "\n";
                            }
                        }
                    }
                }
            }
        }
    }
}

// KICK >>>>>>>>>>>>>>>>>>>>>>>>
void Server::KickCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> split = splitWithSpaces(cleanLine);

    if (!isChanNameValid(split[0]))
    {
        std::cout << "TEST11111\n\n\n";
        sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
    }
    else
    {
        if (!isChannelExist(split[0]))
        {
            std::cout << "TEST22222\n\n\n";
            sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
        }
        else
        {
            if (!_channels[split[0]].isChanMember(_clients[client_fd].getNickname())) // check if the client is in the channel
                sendError(client_fd, ERR_NOTONCHANNEL, split[0]);
            else
            {
                if (split.size() < 2) // check if the command has enough parameters
                {
                    std::cout << "TEST33333\n\n\n";
                    sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
                }
                else
                {
                    int targetFD = findClientFdByUser(split[1]);
                    if (targetFD == -1)
                    {
                        std::cout << "TEST44444\n\n\n";
                        sendError(client_fd, ERR_NOSUCHNICK, split[1]);
                    }
                    else
                    {
                        std::string nick = _clients[targetFD].getNickname(); 
                        std::cout << "=====> "<< nick << std::endl;
                        if (!_channels[split[1]].isChanMember(nick)) // check if the target is in the channel
                        {
                            std::cout << "TEST55555===>split[1]" << split[1] << "\n\n\n";
                            sendError(client_fd, ERR_NOTONCHANNEL, split[1]);
                        }
                        else
                        {
                            _channels[split[0]].removeClient(nick); // remove the target from the channel
                            if (split.size() == 2)                 // check if the command has a message
                            {
                                std::string msg = ":" + _clients[client_fd].getNickname() + " KICK " + split[0] + " " + nick + "\n";
                                send(client_fd, msg.c_str(), msg.size(), 0);
                                msg = ":" + _clients[targetFD].getNickname() + " you got kicked from " + split[0] + "\n";
                                send(targetFD, msg.c_str(), msg.size(), 0);
                            }
                            else // send the message to the target
                            {
                                if (split[1][0] != ':') // check if the message is valid
                                {
                                    std::cout << "TEST66666\n\n\n";
                                    sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
                                }
                                else
                                {
                                    if (split[1][1] == '\0') // check if the message is empty
                                    {
                                        std::string msg = ":" + _clients[client_fd].getNickname() + " KICK " + split[0] + " " + nick + "\n";
                                        send(client_fd, msg.c_str(), msg.size(), 0);
                                    }
                                    else // send the message to the target
                                    {
                                        std::string msg = ": YOU Eject " + nick + " from " + split[0] + "\n";
                                        send(client_fd, msg.c_str(), msg.size(), 0);
                                        msg = ":" + _clients[targetFD].getNickname() + " you got kicked from " + split[0] + " " \
                                        + cleanLine.substr(cleanLine.find(":", split[0].size()) + 1) + "\n"; // get the message after the :
                                        send(targetFD, msg.c_str(), msg.size(), 0);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

// TOPIC >>>>>>>>>>>>>>>>>>>>>>>>
void Server::topicCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> split = splitWithSpaces(cleanLine);

    if (split.size() < 1)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
        std::string nick = _clients[client_fd].getNickname();
        if (!isChanNameValid(split[0]))
            sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
        else
        {
            if (!isChannelExist(split[0]))
                sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
            else
            {
                if (!_channels[split[0]].isChanMember(nick))
                    sendError(client_fd, ERR_NOTONCHANNEL, split[0]);
                else
                {
                    if (split.size() == 1)
                    {
                        std::string msg = ":" + nick + " TOPIC " + split[0] + " :" + _channels[split[0]].getTopic() + "\n";
                        send(client_fd, msg.c_str(), msg.size(), 0);
                    }
                    else
                    {
                        if (split[1][0] != ':')
                            sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
                        else
                        {
                            if (split[1][1] == '\0')
                                _channels[split[0]].setTopic("");
                            else
                            {
                                _channels[split[0]].setTopic(cleanLine.substr(cleanLine.find(":", split[0].size()) + 1));
                                sendWelcomeRpl(client_fd, split[0], RPL_TOPIC, split[1]);
                            }
                        }
                    }
                }
            }
        }
    }
}


// MODE >>>>>>>>>>>>>>>>>>>>>>>>

// void Server::modeCmd(int client_fd, std::string cleanLine)
// {
//     std::vector<std::string> split = splitWithSpaces(cleanLine);

//     if (split.size() < 2)
//         sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
//     else
//     {
//         if (!isChanNameValid(split[0]))
//             sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
//         else
//         {
//             if (!isChannelExist(split[0]))
//                 sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
//             else
//             {
//                 //std::string 
//                 if (join[1] == "+i")
//                     //set the channel to invite only
//                 else if (join[1] == "-i")
//                     //remove Invite-only channel
//                 else if (join[1] == "+t")
//                     //set the restrictions of the TOPIC command to channel operators
//                 else if (join[1] == "-t")
//                     // remove the restrictions of the TOPIC command to channel operators
//                 else if (join[1] == "+k")
//                     // set the channel key (password)
//                 else if (join[1] == "-k")
//                     //remove the channel key (password)
//                 else if (join[1] == "+o")
//                     //Give channel operator privilege
//                 else if (join[1] == "-o")
//                     //Take channel operator privilege
//                 else if (join[1] == "+l")
//                     // set the user limit to channel
//                 else if (join[1] == "-l")
//                     //remove the user limit to channel
//                 else
//                     sendError(client_fd, ERR_UNKNOWNMODE, split[1]);
//             }
//         }
//     }
// }


// Getters ----------------------------------------------------------------------------------------------
int Server::getPort() const { return (this->_port); }         // _port
int Server::getSock_fd() const { return (this->_sock_fd); }   // _sock_fd
std::string Server::getPass() const { return (this->_pass); } // _pass
