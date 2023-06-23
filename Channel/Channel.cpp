#include "Channel.hpp"

Channel::Channel(std::string name, std::string key, std::string chanOpsNick) : _name(name), _key(key), _inviteOnly(false)
{
    _topic = "";
    _chanOps.push_back(chanOpsNick);
}

bool Channel::isChanMember(std::string nick)
{
    for (std::vector<std::string>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (*it == nick)
            return true;
    }
    return false;
}

void Channel::addClient(std::string nick)
{
    _clients.push_back(nick);
}

bool Channel::isChanOps(std::string nick)
{
    for (std::vector<std::string>::iterator it = _chanOps.begin(); it != _chanOps.end(); it++)
    {
        if (*it == nick)
            return true;
    }
    return false;
}


void Channel::removeClient(std::string nick)
{
    for (std::vector<std::string>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (*it == nick)
        {
            _clients.erase(it);
            return;
        }
    }
}