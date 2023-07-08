#include "Client.hpp"

Client::Client(int fd, std::string pass) : _fd(fd), _auth(false), _pass(pass), _registered(false)
{
    _nickname = "";
    _username = "";
    _isOperator = false;
}
