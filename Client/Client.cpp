#include "Client.hpp"

Client::Client(int fd, std::string pass) : _fd(fd), _auth(false), _pass(pass), _registered(false)
{
    _nickname = "";
    _username = "";
    _isOperator = false;

    // get hostname of client
    char host[1024];
    gethostname(host, 1024);
    _hostname = std::string(host);
}
