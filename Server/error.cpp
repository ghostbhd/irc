#include "Server.hpp"

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
	_errorMsg.insert(std::make_pair(421, " :Unknown command\r\n"));
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
		error = ":" + _hostName + " 421 " + nick + " " + command + _errorMsg[error_code]; // :server-name 421 nickname command :Unknown command
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
