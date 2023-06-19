#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../header.h"
#include "../Client/Client.hpp"

class Channel
{
private:
    std::string _name;
    std::vector<Client> _clients;
    std::string _topic;
    std::string _key;
    
};

#endif