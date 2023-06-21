#include "Channel.hpp"

bool Channel::clientExist(int client_fd)
{
    for (std::vector<int>::iterator it = _clients.begin(); it != _clients.end(); it++)
    {
        if (*it == client_fd)
            return true;
    }
    return false;
}