#include "Client.hpp"

Client::Client(int fd, std::string pass) : _fd(fd), _auth(false), _pass(pass)
{
    char host[1024];
    _nickname = "";
    _username = "";
    _isOperator = false;
    gethostname(host, 1024);
    _hostname = std::string(host);
}
