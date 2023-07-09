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

    _sock_fd = socket(AF_INET, SOCK_STREAM, 0); // AF_INET=> IPV4, SOCK_STREAM=>TCP
    _sockaddr.sin_family = AF_INET;
    _sockaddr.sin_addr.s_addr = INADDR_ANY;
    _sockaddr.sin_port = htons(_port);

    if (_sock_fd < 0)
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
    int binding = bind(_sock_fd, (struct sockaddr *)&_sockaddr, sizeof(_sockaddr));
    if (binding < 0) // struct used to specify the @ assigned to the sock
    {
        std::cerr << "Server cannot bind to the address/port \n";
        exit(EXIT_FAILURE);
    }
    int listening = listen(_sock_fd, 100);
    if (listening < 0) // marks a socket as passive, holds at most 100 connections
    {
        std::cerr << "Server cannot listen on socket\n";
        exit(EXIT_FAILURE);
    }
    std::cout << "Server launched !\n";

    char host[1024];
    gethostname(host, 1024);
    _hostName = std::string(host);

    pollfd server_poll;
    memset(&server_poll, 0, sizeof(server_poll));

    server_poll.fd = _sock_fd;
    server_poll.events = POLLIN;

    _poll_vc.push_back(server_poll);
    initErrorMsg();
}

// Main functions ---------------------------------------------------------------------------------------
void Server::start()
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
    ssize_t ReadingFromC = read(client_fd, buffer.data(), buffer.size());
    std::cout << "Client " << client_fd << " sent :\n"
              << buffer.data() << "-----------------------------------------------\n\n";
    if (!ReadingFromC)
    {
        std::cout << "Client " << client_fd << " disconnected!\n";
        _clients.erase(client_fd);

        close(client_fd);
    }

    if (buffer[ReadingFromC - 1] != '\n')
    {
        _clients[client_fd].setBuffer(buffer.data());
        return;
    }
    else
        _clients[client_fd].setBuffer(buffer.data());

    // Getting the line from the buffer and deleting the \n and \r
    std::vector<std::string> split = splitWithChar(_clients[client_fd].getBuffer(), '\n');
    _clients[client_fd].eraseBuffer();

    for (std::vector<std::string>::iterator it = split.begin(); it != split.end(); it++)
    {
        std::string CleanLine = deleteNewLine(*it);
        if (CleanLine.empty())
            return;

        size_t pos = CleanLine.find(" "); // Getting the position of the first space
        // check if the line is valid
        if (pos == std::string::npos || pos == 0 || CleanLine[pos + 1] == '\0')
        {
            sendError(client_fd, ERR_NEEDMOREPARAMS, CleanLine);
            return;
        }

        // Parameter before " " ex: NICK, USER, PASS, etc...
        std::string cmd = CleanLine.substr(0, pos);

        // Pass if passwd is correct then auth = true

        if (cmd == "CAP")
            continue;

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
                    std::vector<std::string> nick = splitWithChar(CleanLine.substr(pos + 1), ' ');
                    if (nick.size() != 1)
                        sendError(client_fd, ERR_NEEDMOREPARAMS, CleanLine);
                    else if (findClientFdByNick(nick[0]) != -1)
                        sendError(client_fd, ERR_NICKNAMEINUSE, nick[0]);
                    else
                    {
                        if (_clients[client_fd].getNickname().empty())
                        {
                            _clients[client_fd].setNickname(nick[0]);
                            std::string msg = ":" + _clients[client_fd].getNickname() + " NICK " + _clients[client_fd].getNickname() + "\r\n";
                            send(client_fd, msg.c_str(), msg.size(), 0);
                        }
                        else
                            sendError(client_fd, ERR_NICKNAMEINUSE, CleanLine);
                    }
                }
                else if (cmd == "USER" || cmd == "user")
                {
                    std::vector<std::string> user = splitWithChar(CleanLine.substr(pos + 1), ' ');

                    if (_clients[client_fd].getUsername().empty())
                        _clients[client_fd].setUsername(user[0]);
                    else
                        sendError(client_fd, ERR_ALREADYREGISTERED, CleanLine);
                }
                else
                    sendError(client_fd, ERR_NOTREGISTERED, CleanLine);

                // std::cout << "client user: " << _clients[client_fd].getUsername() << std::endl;

                if (!_clients[client_fd].getUsername().empty() && !_clients[client_fd].getNickname().empty())
                {
                    sendWelcomeRpl(client_fd, _clients[client_fd].getNickname());
                    std::cout << "Client [" << _clients[client_fd].getNickname() << "] is now registered\n";
                }
            }
            else
            {
                mainCommands(client_fd, &CleanLine[pos + 1], cmd);
            }
        }
    }
}

// Utils ------------------------------------------------------------------------------------------------
std::string Server::deleteNewLine(std::string str)
{
    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == '\n' || str[i] == '\r')
            str.erase(i, 1);
    }

    return str;
}

std::vector<std::string> Server::splitWithChar(std::string str, char c)
{
    std::vector<std::string> result;
    std::string tmp;

    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] == c)
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

void Server::closeClient(int client_fd)
{
    // receive part for all the channels the client is in
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
    {
        if (it->second.isChanMember(_clients[client_fd].getNickname()))
        {
            partCmd(client_fd, it->first);
        }
    }
    close(client_fd);
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

void Server::MsgToChannel(std::string chanName, std::string msg, int client_fd)
{
    std::string nick = _clients[client_fd].getNickname();
    // [channel_name] <sender_nick> message_content
    std::string message = ":" + _clients[client_fd].getNickname() + "!~h@" + _hostName + " PRIVMSG " + chanName + " :" + msg + "\n";
    ;
    std::vector<std::string> clients = _channels[chanName].getMembers();
    for (std::vector<std::string>::iterator it = clients.begin(); it != clients.end(); it++)

    {
        int fd = findClientFdByNick(*it);
        if (fd != client_fd)
            send(fd, message.c_str(), message.size(), 0);
    }
}

// Send Replay ------------------------------------------------------------------------------------------
void Server::sendWelcomeRpl(int client_fd, std::string nick)
{
    // :server-name 001 your_nickname :Welcome to the IRC Network, your_nickname!user@host
    std::string username = _clients[client_fd].getUsername();
    std::string message = ":" + _hostName + " 001 " + nick + " :Welcome to the IRC Network, " + nick + "!" + username + "@" + _hostName + "\r\n";
    send(client_fd, message.c_str(), message.size(), 0);
}

// Error handling ---------------------------------------------------------------------------------------
void Server::initErrorMsg()
{
    _errorMsg.insert(std::make_pair(433, " :Nickname is already in use\r\n"));
    _errorMsg.insert(std::make_pair(443, " :is already on channel\r\n"));
    _errorMsg.insert(std::make_pair(451, " :You have not registered\r\n"));
    _errorMsg.insert(std::make_pair(461, " :Not enough parameters\r\n"));
    _errorMsg.insert(std::make_pair(464, " :Password incorrect\r\n"));
    _errorMsg.insert(std::make_pair(501, " :Unknown MODE flag\r\n"));
    _errorMsg.insert(std::make_pair(462, " :You may not reregister\r\n"));
    _errorMsg.insert(std::make_pair(401, " :No such nick/channel\r\n"));
    _errorMsg.insert(std::make_pair(407, " :Too many targets\r\n"));
    _errorMsg.insert(std::make_pair(403, " :No such channel\r\n"));
    _errorMsg.insert(std::make_pair(475, " :Cannot join channel (+k) - Bad channel key\r\n"));
    _errorMsg.insert(std::make_pair(473, " :Cannot join channel (+i)\r\n")); // new
    _errorMsg.insert(std::make_pair(405, " :You are already registred\r\n"));
    _errorMsg.insert(std::make_pair(482, " :You're not channel operator\r\n"));
    _errorMsg.insert(std::make_pair(442, " :You're not on that channel\r\n"));
    _errorMsg.insert(std::make_pair(472, " :is unknown mode char to me\r\n"));
    _errorMsg.insert(std::make_pair(421, " :bot does not recognize the command\r\n"));
    _errorMsg.insert(std::make_pair(411, " :No recipient given\r\n"));
    _errorMsg.insert(std::make_pair(404, " :Cannot send to channel\r\n"));
    _errorMsg.insert(std::make_pair(471, " :Cannot join channel (+l)\r\n"));
    _errorMsg.insert(std::make_pair(481, " :Permission Denied - You do not have the necessary privileges\r\n"));
}

void Server::sendError(int client_fd, int error_code, std::string command) // need to add channel name to error msg
{
    std::string error;
    std::string fd_string;
    std::stringstream ss;
    ss << client_fd;
    ss >> fd_string;

    std::string nick = _clients[client_fd].getNickname();

    if (error_code == 433)
        error = ":" + _hostName + " 433 * " + command + _errorMsg[error_code]; // :server-name 433 * your_nickname :Nickname is already in use.
    else if (error_code == 443)
        error = nick + _errorMsg[error_code]; // your-nickname :is already on channel\r\n
    else if (error_code == 451)
        error = ":" + _hostName + " 451 " + command + _errorMsg[error_code]; //: irc.server.com 451 JOIN :You have not registered\r\n";
    else if (error_code == 461)
        error = ":" + _hostName + " 461 " + nick + _errorMsg[error_code]; // :server-name 461 your-nickname :Not enough parameters\r\n
    else if (error_code == 464)
        error = ":" + _hostName + " 464 " + nick + _errorMsg[error_code]; // :server-name 464 your-nickname :Password incorrect\r\n

    else if (error_code == 501)
        error = fd_string + _errorMsg[error_code];
    else if (error_code == 462)
        error = fd_string + _errorMsg[error_code];
    else if (error_code == 401)
        error = ":" + _hostName + " 401 " + nick + _errorMsg[error_code]; // :server-name 401 your-nickname :No such nick/channel\r\n
    else if (error_code == 407)
        error = fd_string + " " + _errorMsg[error_code];
    else if (error_code == 403)
        error = ":" + _hostName + " 403 " + nick + _errorMsg[error_code]; // :server-name 403 your-nickname :No such channel\r\n
    else if (error_code == 475)
        error = ":" + _hostName + " 475 " + nick + " " + command + _errorMsg[error_code]; // :server-name 475 <your-nickname> <channel-name> :Cannot join channel (+k) - Bad channel key\r\n
    else if (error_code == 405)
        error = fd_string + " " + command + _errorMsg[error_code];
    else if (error_code == 482)
        error = ":" + _hostName + " 482 " + nick + _errorMsg[error_code]; // :server-name 482 your-nickname :You're not channel operator\r\n
    else if (error_code == 442)
        error = ":" + _hostName + " 442 " + nick + _errorMsg[error_code]; // :server-name 442 your-nickname :You're not on that channel\r\n
    else if (error_code == 472)
        error = ":" + _hostName + " 472 " + nick + _errorMsg[error_code]; // :server-name 472 your-nickname :Unknown mode char to me\r\n
    else if (error_code == 421)
        error = fd_string + " " + command + _errorMsg[error_code];
    else if (error_code == 411)
        error = ":" + _hostName + " 411 " + nick + _errorMsg[error_code]; //: irc.server.com 411 YourNickname :No recipient given (PRIVMSG)
    else if (error_code == 404)
        error = ":" + _hostName + " 404 " + nick + " " + command + _errorMsg[error_code]; //: irc.server.com ERR_CANNOTSENDTOCHAN YourNickname #channel :Cannot send to channel
    else if (error_code == 473)
        error = ":" + _hostName + " 473 " + nick + " " + command + _errorMsg[error_code]; // :server-name 473 <your-nickname> <channel-name> :Cannot join channel (+i)
    else if (error_code == 471)
        error = ":" + _hostName + " 471 " + nick + " " + command + _errorMsg[error_code]; // :server-name 471 <your-nickname> <channel-name> :Cannot join channel (+l)
    else if (error_code == 481) 
        error = ":" + _hostName + " 481 " + nick + _errorMsg[error_code]; // :server-name 481 nickname :Permission Denied - You do not have the necessary privileges

    std::cout << "Error: " << error << std::endl;
    send(client_fd, error.c_str(), error.size(), 0);
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
    else if (cmd == "MODE")
        modeCmd(client_fd, cleanLine);
    else if (cmd == "PING")
        pingCmd(client_fd, cleanLine);
    else if (cmd == "BOT")
        botCmd(client_fd, cleanLine);
    else if (cmd == "NOTICE")
        noticeCmd(client_fd, cleanLine);
    else if (cmd == "PART")
        partCmd(client_fd, cleanLine);
    else if (cmd == "WHOIS")
        return;
    else if (cmd == "QUIT")
        quitCmd(client_fd, cleanLine);
    else if (cmd == "KILL" || cmd == "kill")
        killCmd(client_fd, cleanLine);
    else
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
}

// OPER >>>>>>>>>>>>>>>>>>>>>>>>
void Server::operCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> oper = splitWithChar(cleanLine, ' ');
    if (oper.size() != 2)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
        if (oper[0] == _adminName && oper[1] == _adminPass)
        {
            _clients[client_fd].setOperator(true);
            // :server-name MODE <your-nickname> :+o

            std::string msg = ":" + _hostName + " MODE " + _clients[client_fd].getNickname() + " :+o\r\n";
            send(client_fd, msg.c_str(), msg.size(), 0);
        }
        else
            sendError(client_fd, ERR_PASSWDMISMATCH, cleanLine);
    }
}

// PRIVMSG >>>>>>>>>>>>>>>>>>>>
void Server::privmsg(int client_fd, std::string cleanLine)
{
    std::vector<std::string> msg = splitWithChar(cleanLine, ' ');
    if (msg.size() < 2)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
        if (msg[0][0] == '#')
        {
            if (isChannelExist(msg[0]))
            {
                if (!_channels[msg[0]].isChanMember(_clients[client_fd].getNickname()))
                    sendError(client_fd, ERR_NOTONCHANNEL, msg[0]);
                else
                {
                    if (msg[1][0] != ':')
                        sendError(client_fd, ERR_TOOMANYTARGETS, cleanLine);
                    else
                        MsgToChannel(msg[0], cleanLine.substr(cleanLine.find(":", msg[0].size()) + 1), client_fd);
                }
            }
            else
                sendError(client_fd, ERR_NOSUCHCHANNEL, cleanLine);
        }
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
                    // <sender> <message>
                    // std::string msg = _clients[client_fd].getNickname() + " " + cleanLine.substr(cleanLine.find(":") + 1) + "\n";
                    std::string msg = ":" + _clients[client_fd].getNickname() + "!~h@" + _hostName + " PRIVMSG " + _clients[fd].getNickname() + " :" + cleanLine.substr(cleanLine.find(":") + 1) + "\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            }
        }
    }
}

// JOIN >>>>>>>>>>>>>>>>>>>>>>>>
void Server::joinCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> join = splitWithChar(cleanLine, ' '); // split line with spaces

    std::string nick = _clients[client_fd].getNickname(); // get client nickname

    std::string chanName = join[0]; // get channel name

    if (!isChanNameValid(chanName)) // channel name must be valid
        sendError(client_fd, ERR_NOSUCHCHANNEL, chanName);
    else
    {
        std::string key = "";
        if (join.size() > 1)
            key = cleanLine.substr(cleanLine.find(join[1], chanName.length())); // get key

        if (!isChannelExist(chanName)) // channel not found create it and add client
        {
            Channel newChannel(chanName, key, nick);                // create new channel (key, name, chanOps)
            _channels.insert(std::make_pair(chanName, newChannel)); // add channel to map
            _channels[chanName].addMember(nick);                    // add chanops to channel as client

            std::string msg = ":" + nick + " JOIN " + chanName + "\n";
            send(client_fd, msg.c_str(), msg.size(), 0);

            std::cout << "New channel created : " << chanName << " by " << nick << std::endl;
        }
        else // channel found add client to it
        {
            if (_channels[chanName].getInviteOnly() && !_channels[chanName].isInviteList(nick)) // channel is invite only and client is not in invite list
                sendError(client_fd, ERR_INVITEONLYCHAN, chanName);
            else
            {
                if (_channels[chanName].getLimit() && _channels[chanName].getMembers().size() >= _channels[chanName].getLimit()) // channel is full
                    sendError(client_fd, ERR_CHANNELISFULL, chanName);
                else
                {
                    if (_channels[chanName].getKey().empty()) // no key
                    {
                        if (_channels[chanName].isChanMember(nick)) // client already in channel
                            sendError(client_fd, ERR_TOOMANYCHANNELS, chanName);
                        else // add client to channel (no key)
                        {
                            if (!key.empty())
                                sendError(client_fd, ERR_BADCHANNELKEY, chanName);
                            else
                            {

                                _channels[chanName].addMember(nick);
                                std::string msg = ":" + nick + " JOIN " + chanName + "\r\n";
                                send(client_fd, msg.c_str(), msg.size(), 0);

                                // ======> send join msg to all channel ops
                                // :<join-nickname>!<join-username>@<join-hostname> JOIN <channel-name>
                                msg = ":" + nick + "!" + _clients[client_fd].getUsername() + "@" + _hostName + " JOIN " + chanName + "\r\n";
                                for (std::vector<std::string>::iterator it = _channels[chanName].getChanOps().begin(); it != _channels[chanName].getChanOps().end(); ++it)
                                {
                                    int fd = findClientFdByNick(*it);
                                    if (fd != -1)
                                        send(fd, msg.c_str(), msg.size(), 0);
                                }
                            }
                        }
                    }
                    else // key needed
                    {
                        if (_channels[chanName].getKey() == key) // key match add client
                        {
                            _channels[chanName].addMember(nick);
                            std::string msg = ":" + nick + " JOIN " + chanName + "\r\n";
                            send(client_fd, msg.c_str(), msg.size(), 0);
                        }
                        else
                            sendError(client_fd, ERR_BADCHANNELKEY, chanName);
                    }
                }
            }
        }
    }
}

// INVITE >>>>>>>>>>>>>>>>>>>>>>>>
void Server::inviteCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> inv = splitWithChar(cleanLine, ' ');

    std::string inviter = _clients[client_fd].getNickname(); // get client nickname

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
                if (_channels[inv[1]].getInviteOnly() && !_channels[inv[1]].isChanOps(inviter))
                    sendError(client_fd, ERR_CHANOPRIVSNEEDED, inv[1]);
                else
                {
                    int invitedFd = findClientFdByNick(inv[0]);
                    if (invitedFd == -1) // check if the client exists in the server
                        sendError(client_fd, ERR_NOSUCHNICK, inv[0]);
                    else
                    {
                        std::string invitedNick = _clients[invitedFd].getNickname();
                        if (!_channels[inv[1]].isChanMember(inviter)) // check if the inviter is in the channel
                            sendError(client_fd, ERR_NOTONCHANNEL, inv[1]);
                        else
                        {
                            if (_channels[inv[1]].isChanMember(invitedNick)) // check if the invited client is already in the channel
                                sendError(client_fd, ERR_USERONCHANNEL, inv[1]);
                            else
                            {
                                // add client to ivited list
                                _channels[inv[1]].addInviteList(invitedNick);
                                // :inviter_nick INVITE your_nick :#channel
                                std::string msg = ":" + inviter + " INVITE " + invitedNick + " :" + inv[1] + "\r\n";
                                send(invitedFd, msg.c_str(), msg.size(), 0);
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
    std::vector<std::string> split = splitWithChar(cleanLine, ' ');

    if (!isChanNameValid(split[0]))
    {
        sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
    }
    else
    {
        if (!isChannelExist(split[0]))
        {
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
                    sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
                }
                else
                {
                    int targetFD = findClientFdByUser(split[1]);
                    if (targetFD == -1)
                    {
                        sendError(client_fd, ERR_NOSUCHNICK, split[1]);
                    }
                    else
                    {
                        std::string nick = _clients[targetFD].getNickname();
                        if (!_channels[split[0]].isChanMember(nick)) // check if the target is in the channel
                        {
                            sendError(client_fd, ERR_NOTONCHANNEL, split[1]);
                        }
                        else
                        {
                            _channels[split[0]].removeMember(nick); // remove the target from the channel
                            if (split.size() == 2)                  // check if the command has a message
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
                                        msg = ":" + _clients[targetFD].getNickname() + " you got kicked from " + split[0] + " " + cleanLine.substr(cleanLine.find(":", split[0].size()) + 1) + "\n"; // get the message after the :
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
    std::vector<std::string> split = splitWithChar(cleanLine, ' ');
    std::string nick = _clients[client_fd].getNickname();

    if (split.size() < 1)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
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
                    if (split.size() == 1) // check if the command has a message or not (if not send the topic)
                    {
                        // :server-name 332 <your-nickname> <channel-name> :<topic>
                        std::string msg = ":" + _hostName + " 332 " + nick + " " + split[0] + " :" + _channels[split[0]].getTopic() + "\r\n";

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

                                // :server-name 332 <your-nickname> <channel-name> :<topic>
                                std::string msg = ":" + _hostName + " 332 " + nick + " " + split[0] + " :" + _channels[split[0]].getTopic() + "\r\n";
                                MsgToChannel(split[0], msg, client_fd);
                                send(client_fd, msg.c_str(), msg.size(), 0);
                            }
                        }
                    }
                }
            }
        }
    }
}

// MODE >>>>>>>>>>>>>>>>>>>>>>>>
void Server::modeCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> split = splitWithChar(cleanLine, ' ');

    if (split.size() < 2)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine); // repl insted of error
    else
    {
        if (!isChanNameValid(split[0]))
            return;
        else
        {
            if (!isChannelExist(split[0]))
                sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
            else
            {
                if (_channels[split[0]].isChanOps(_clients[client_fd].getNickname()))
                {
                    if (split[1] == "+i")
                        _channels[split[0]].setInviteOnly(true);
                    else if (split[1] == "-i")
                        _channels[split[0]].setInviteOnly(false);
                    else if (split[1] == "+t")
                        _channels[split[0]].setTopicRestriction(true);
                    else if (split[1] == "-t")
                        _channels[split[0]].setTopicRestriction(false);
                    else if (split[1] == "+k" && split.size() == 3)
                        _channels[split[0]].setKey(split[2]);
                    else if (split[1] == "-k")
                        _channels[split[0]].setKey("");
                    else if (split[1] == "+o")
                        _channels[split[0]].addChanOps(split[2]);
                    else if (split[1] == "-o")
                        _channels[split[0]].removeChanOps(split[2]);
                    else if (split[1] == "+l")
                    {
                        _channels[split[0]].setLimit(true);
                        int value = std::stoi(split[2]);
                        if (value < 0)
                            sendError(client_fd, ERR_UNKNOWNMODE, "");
                        else
                        {
                            if (split.size() == 3)
                                _channels[split[0]].setLimitValue(value);
                            else
                                _channels[split[0]].setLimit(false);
                        }
                    }
                    else if (split[1] == "-l")
                        _channels[split[0]].setLimit(false);
                    else
                        sendError(client_fd, ERR_UNKNOWNMODE, split[1]);
                }
                else
                    sendError(client_fd, ERR_CHANOPRIVSNEEDED, split[0]);
            }
        }
    }
}

// PING >>>>>>>>>>>>>>>>>>>>>>>>
void Server::pingCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> split = splitWithChar(cleanLine, ' ');

    std::string msg = "PONG " + split[0] + "\n";
    send(client_fd, msg.c_str(), msg.size(), 0);
}

// NOTICE >>>>>>>>>>>>>>>>>>>>>>>>
void Server::noticeCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> notif = splitWithChar(cleanLine, ' ');
    if (notif.size() < 2)
        sendError(client_fd, ERR_NORECIPIENT, cleanLine);
    else
    {
        if (notif[0][0] == '#')
        {
            if (isChannelExist(notif[0]))
            {
                if (!_channels[notif[0]].isChanMember(_clients[client_fd].getNickname()))
                    sendError(client_fd, ERR_CANNOTSENDTOCHAN, notif[0]);
                else
                {
                    if (notif[1][0] != ':')
                        sendError(client_fd, ERR_TOOMANYTARGETS, cleanLine);
                    else
                        MsgToChannel(notif[0], cleanLine.substr(cleanLine.find(":", notif[0].size()) + 1), client_fd);
                }
            }
            else
                sendError(client_fd, ERR_NOSUCHCHANNEL, cleanLine);
        }
        else
        {
            int fd = findClientFdByNick(notif[0]);
            if (fd == -1)
                sendError(client_fd, ERR_NOSUCHNICK, cleanLine);
            else
            {
                if (notif[1][0] != ':')
                    sendError(client_fd, ERR_TOOMANYTARGETS, cleanLine);
                else
                {
                    std::string msg = ":" + _clients[client_fd].getNickname() + " NOTICE " + notif[0] + " :" + cleanLine.substr(cleanLine.find(":", notif[0].size()) + 1) + "\n";
                    send(fd, msg.c_str(), msg.size(), 0);
                }
            }
        }
    }
}

// PART >>>>>>>>>>>>>>>>>>>>>>>>
void Server::partCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> split = splitWithChar(cleanLine, ' ');

    if (split.size() > 1)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    if (!isChanNameValid(split[0]))
        sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
    else if (!isChannelExist(split[0]) && !_channels[split[0]].isChanMember(_clients[client_fd].getNickname()))
        sendError(client_fd, ERR_NOTONCHANNEL, split[0]);
    else
    {
        _channels[split[0]].removeMember(_clients[client_fd].getNickname());
        std::string msg = ":" + _clients[client_fd].getNickname() + " PART " + split[0] + "\n";
        send(client_fd, msg.c_str(), msg.size(), 0);
    }
}

// BOT >>>>>>>>>>>>>>>>>>>>>>>>
void Server::botCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> split = splitWithChar(cleanLine, ' ');
    if (split.size() < 1)
    {
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
        return;
    }
    else
    {
        if (split[0] == "Version")
        {
            std::string msg = ":" + _clients[client_fd].getNickname() + " BOT Version 1.0\n";
            send(client_fd, msg.c_str(), msg.size(), 0);
        }
        else if (split[0] == "Info")
        {
            std::stringstream ss;
            std::string port;
            ss << getPort();
            ss >> port;
            std::string msg = ": Infos of " + _clients[client_fd].getNickname() + "\n" + "Host: " + _hostName + "\n" + "Port: " + port + "\n" + "Server: IRC server\n";
            send(client_fd, msg.c_str(), msg.size(), 0);
        }
        else if (split[0] == "Time")
        {
            time_t now = time(0);
            tm *ltm = localtime(&now);
            std::stringstream oss;
            std::string hour, min, sec;
            oss << ltm->tm_hour << ":" << ltm->tm_min << ":" << ltm->tm_sec;
            oss >> hour >> min >> sec;
            std::string msg = ":" + _clients[client_fd].getNickname() + " BOT Time " + hour + min + sec + "\n";
            send(client_fd, msg.c_str(), msg.size(), 0);
        }
        else
            sendError(client_fd, ERR_UNKNOWNCOMMAND, split[0]);
    }
}

// QUIT >>>>>>>>>>>>>>>>>>>>>>>>
void Server::quitCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> split = splitWithChar(cleanLine, ' ');

    if (split.size() > 1)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
        if (split[0][0] != ':')
            sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
        else
        {
            // :<quitting-nickname>!<quitting-username>@<quitting-hostname> QUIT :<quit-message>
            if (_channels.size())
            {
                std::string msg = ":" + _clients[client_fd].getNickname() + "!" + _clients[client_fd].getUsername() + "@" + _hostName + " QUIT :" + cleanLine + "\r\n";
                for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) // send msg to all channels where client is member
                {
                    for (std::vector<std::string>::iterator it2 = it->second.getMembers().begin(); it2 != it->second.getMembers().end(); ++it2)
                    {
                        if (*it2 == _clients[client_fd].getNickname())
                        {
                            MsgToChannel(it->first, msg, client_fd);
                            _channels[it->first].removeMember(_clients[client_fd].getNickname());
                            break;
                        }
                    }
                }
            }
            close(client_fd);
            _clients.erase(client_fd);
        }
    }
}

// KILL >>>>>>>>>>>>>>>>>>>>>>>>
void Server::killCmd(int client_fd, std::string cleanLine)
{
    std::vector<std::string> split = splitWithChar(cleanLine, ' ');

    if (split.size() < 2)
        sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
    else
    {
        std::string targetNick = split[0];

        if (!_clients[client_fd].getOperator())
            sendError(client_fd, ERR_NOPRIVILEGES, cleanLine);
        else
        {
            if (split[1][0] != ':')
                sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
            else
            {
                int fd = findClientFdByNick(split[0]);
                if (fd == -1)
                    sendError(client_fd, ERR_NOSUCHNICK, split[1]);
                else
                {
                    std::string reason = cleanLine.substr(cleanLine.find(':', split[0].size()) + 1);
                    std::string nick = _clients[client_fd].getNickname();
                    
                    // to operator => :server-name 361 operator-nickname target-nickname :User has been killed (reason)
                    std::string msgToOp = ":" + _hostName + " 361 " + nick + " " + split[0] + " :User has been killed (" + reason + ")\r\n";
                    send(client_fd, msgToOp.c_str(), msgToOp.size(), 0);

                    // to target => :operator-nickname KILL target-nickname :You have been killed (reason)
                    std::string msgToTarget = ":" + nick + " KILL " + split[0] + " :You have been killed (" + reason + ")\r\n";
                    send(fd, msgToTarget.c_str(), msgToTarget.size(), 0);
                    closeClient(fd);
                }
            }
        }
    }
}

// Getters ----------------------------------------------------------------------------------------------
int Server::getPort() const { return (_port); }         // _port
int Server::getSock_fd() const { return (_sock_fd); }   // _sock_fd
std::string Server::getPass() const { return (_pass); } // _pass

// Destructor -------------------------------------------------------------------------------------------
Server::~Server()
{
    std::map<int, Client>::iterator it = _clients.begin();
    while (++it != _clients.end())
        closeClient(it->first);
    close(_sock_fd);
    std::cout << "Server is OFF !\n";
}
