#include "Channel.hpp"

Channel::Channel(std::string name, std::string key, int chanOps) : _name(name), _key(key)
{
    _topic = "";
    _chanOps.push_back(chanOps);
}

bool Channel::clientExist(int client_fd)
{
    for (std::vector<int>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (*it == client_fd)
            return true;
    }
    return false;
}

void Channel::addClient(int client_fd)
{
    _clients.push_back(client_fd);
}

