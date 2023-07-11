#include "Server.hpp"

// Main commands _________________________________________
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

// OPER _________________________________________
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

// JOIN _________________________________________
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
			Channel newChannel(chanName, key, nick);				// create new channel (key, name, chanOps)
			_channels.insert(std::make_pair(chanName, newChannel)); // add channel to map
			_channels[chanName].addMember(nick);					// add chanops to channel as client

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

// INVITE _________________________________________
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

// KICK _________________________________________
void Server::KickCmd(int client_fd, std::string cleanLine)
{
	std::vector<std::string> split = splitWithChar(cleanLine, ' ');

	if (!_channels[split[0]].isChanOps(_clients[client_fd].getNickname()))
	{
		sendError(client_fd, ERR_CHANOPRIVSNEEDED, split[0]);
		return;
	}

	if (!isChanNameValid(split[0]))
		sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
	else
	{
		if (!isChannelExist(split[0]))
			sendError(client_fd, ERR_NOSUCHCHANNEL, split[0]);
		else
		{
			if (!_channels[split[0]].isChanMember(_clients[client_fd].getNickname())) // check if the client is in the channel
				sendError(client_fd, ERR_NOTONCHANNEL, split[0]);
			else
			{
				if (split.size() < 3) // check if the command has enough parameters
					sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
				else
				{
					std::string kicked_nick = split[1];
					int targetFD = findClientFdByNick(kicked_nick);
					if (targetFD == -1)
					{
						sendError(client_fd, ERR_NOSUCHNICK, kicked_nick);
						std::cout << "channel name not valid" << std::endl;
					}
					else
					{
						if (!_channels[split[0]].isChanMember(kicked_nick)) // check if the target is in the channel
							sendError(client_fd, ERR_NOTONCHANNEL, kicked_nick);
						else
						{
							if (split[2][0] != ':') // check if the message is valid
								sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
							else
							{
								partCmd(targetFD, split[0]);
								std::string kicker_nick = _clients[client_fd].getNickname();
								// :<kicker_nick> KICK <channel> <kicked_nick> :<reason>
								std::string msg = ":" + kicker_nick + " KICK " + split[0] + " " + kicked_nick + " " + cleanLine.substr(cleanLine.find(":", split[0].size() + split[1].size())) + "\n";
								MsgToChannel(split[0], msg, client_fd);
							}
						}
					}
				}
			}
		}
	}
}

// TOPIC _________________________________________
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

// MODE _________________________________________
void Server::modeCmd(int client_fd, std::string cleanLine)
{
	std::vector<std::string> split = splitWithChar(cleanLine, ' ');
	bool isOk = true;

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
					if (split[1] == "+i" && split.size() == 2)
						_channels[split[0]].setInviteOnly(true);
					else if (split[1] == "-i" && split.size() == 2)
						_channels[split[0]].setInviteOnly(false);
					else if (split[1] == "+t")
						_channels[split[0]].setTopicRestriction(true);
					else if (split[1] == "-t" && split.size() == 2)
						_channels[split[0]].setTopicRestriction(false);
					else if (split[1] == "+k" && split.size() == 3)
					{
						_channels[split[0]].setKey(split[2]);
					}
					else if (split[1] == "-k" && split.size() == 2)
						_channels[split[0]].setKey("");
					else if (split[1] == "+o")
					{
						if (!_channels[split[0]].isChanMember(split[2]))
							sendError(client_fd, ERR_NOTONCHANNEL, split[2]);
						else
						{
							_channels[split[0]].addChanOps(split[2]);
							// :operator-nickname MODE #channel +o other-user-nickname
							std::string msg = ":" + _clients[client_fd].getNickname() + " MODE " + split[0] + " +o " + split[2] + "\r\n";
							MsgToChannel(split[0], msg, client_fd);
							send(client_fd, msg.c_str(), msg.size(), 0);
						}
					}
					else if (split[1] == "-o")
					{
						if (!_channels[split[0]].isChanOps(split[2]))
							sendError(client_fd, ERR_NOTONCHANNEL, split[2]);
						else
						{
							_channels[split[0]].removeChanOps(split[2]);
							// :operator-nickname MODE #channel -o other-user-nickname
							std::string msg = ":" + _clients[client_fd].getNickname() + " MODE " + split[0] + " -o " + split[2] + "\r\n";
							MsgToChannel(split[0], msg, client_fd);
						}
					}
					else if (split[1] == "+l")
					{
						_channels[split[0]].setLimit(true);
						int value = atoi(split[2].c_str());
						if (value < 0)
						{
							isOk = false;
							sendError(client_fd, ERR_UNKNOWNMODE, "");
						}
						else
						{
							if (split.size() == 3)
								_channels[split[0]].setLimitValue(value);
							else
								_channels[split[0]].setLimit(false);
						}
					}
					else if (split[1] == "-l" && split.size() == 2)
						_channels[split[0]].setLimit(false);
					else
					{
						isOk = false;
						sendError(client_fd, ERR_UNKNOWNMODE, split[1]);
					}
					if (isOk && split[1] != "+o" && split[1] != "-o")
					{
						// :operator-nickname MODE #channel +i
						std::string msg = ":" + _clients[client_fd].getNickname() + " MODE " + split[0] + " " + split[1] + "\r\n";
						std::vector<std::string> chanOps = _channels[split[0]].getChanOps();
						for (size_t i = 0; i < chanOps.size(); i++)
						{
							if (chanOps[i] != _clients[client_fd].getNickname())
								send(findClientFdByNick(chanOps[i]), msg.c_str(), msg.size(), 0);
						}
					}
				}
				else
					sendError(client_fd, ERR_CHANOPRIVSNEEDED, split[0]);
			}
		}
	}
}

// PING _________________________________________
void Server::pingCmd(int client_fd, std::string cleanLine)
{
	std::string msg = "PONG " + cleanLine + "\r\n";
	send(client_fd, msg.c_str(), msg.size(), 0);
}

// NOTICE _________________________________________
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

// PART _________________________________________

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
		// MsgToChannel(split[0], msg, client_fd);˚∆
		send(client_fd, msg.c_str(), msg.size(), 0);
	}
}

// BOT _________________________________________
void Server::botCmd(int client_fd, std::string cleanLine)
{
	std::vector<std::string> split = splitWithChar(cleanLine, ' ');
	if (split.size() != 1)
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

// QUIT _________________________________________
void Server::quitCmd(int client_fd, std::string cleanLine)
{

	if (cleanLine[0] != ':')
		sendError(client_fd, ERR_NEEDMOREPARAMS, cleanLine);
	else
	{
		// :<quitting-nickname>!<quitting-username>@<quitting-hostname> QUIT :<quit-message>
		if (_channels.size())
		{

			std::string msg = ":" + _clients[client_fd].getNickname() + "!" + _clients[client_fd].getUsername() + "@" + _hostName + " QUIT " + cleanLine + "\r\n";
			for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) // send msg to all channels where client is member
			{
				std::vector<std::string> members = it->second.getMembers();
				for (std::vector<std::string>::iterator it2 = members.begin(); it2 != members.end(); ++it2)
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
		std::map<int, Client>::iterator it = _clients.find(client_fd);
		if (it != _clients.end())
			_clients.erase(it);
	}
}

// KILL _________________________________________
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
					closeClient(fd); // close target client and send part msg to all channels where target is member
				}
			}
		}
	}
}
