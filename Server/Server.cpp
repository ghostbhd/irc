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
int Server::start()
{
	if (poll(_poll_vc.data(), _poll_vc.size(), 0) < 0)
	{
		std::cerr << "Cannot connect with multiple clients\n";
		return 1;
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
	return 0;
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
	if (!ReadingFromC) // if the client disconnected
	{
		std::cout << "Client " << client_fd << " disconnected!\n";
		quitCmd(client_fd, ":disconnected");
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

// Send Replay ------------------------------------------------------------------------------------------
void Server::sendWelcomeRpl(int client_fd, std::string nick)
{
	// :server-name 001 your_nickname :Welcome to the IRC Network, your_nickname!user@host
	std::string username = _clients[client_fd].getUsername();
	std::string message = ":" + _hostName + " 001 " + nick + " :Welcome to the IRC Network, " + nick + "!" + username + "@" + _hostName + "\r\n";
	send(client_fd, message.c_str(), message.size(), 0);
}

// Getters ----------------------------------------------------------------------------------------------
int Server::getPort() const { return (_port); }			// _port
int Server::getSock_fd() const { return (_sock_fd); }	// _sock_fd
std::string Server::getPass() const { return (_pass); } // _pass

// Destructor -------------------------------------------------------------------------------------------
Server::~Server()
{
	std::cout << "Server is shutting down...\n";
	while (!_clients.empty())
	{
		std::map<int, Client>::iterator it = _clients.begin();
		closeClient(it->first);
	}
	
	close(_sock_fd);
	std::cout << "Server is OFF !\n";
}
