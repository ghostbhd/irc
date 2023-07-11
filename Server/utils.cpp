#include "Server.hpp"

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

void Server::closeClient(int client_fd)
{
	// receive part for all the channels the client is in
	if (_channels.size() > 0)
	{
		for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); it++)
		{
			if (it->second.isChanMember(_clients[client_fd].getNickname()))
			{
				partCmd(client_fd, it->first);
			}
		}
	}
	close(client_fd);
	std::map<int, Client>::iterator it = _clients.find(client_fd);
	if (it != _clients.end())
		_clients.erase(it);
	std::cout << "Client " << client_fd << " is now deleted\n";
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
