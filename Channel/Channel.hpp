#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../header.h"

class Channel
{
private:
    std::string _name;
    std::vector<int> _clients;
    std::string _topic;
    std::string _key;
public:
    Channel() {}
    bool clientExist(int client_fd);
};

#endif