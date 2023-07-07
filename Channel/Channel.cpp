#include "Channel.hpp"

Channel::Channel(std::string name, std::string key, std::string chanOpsNick) : _name(name), _key(key), _inviteOnly(false), _topicRestriction(false), _limit(false)
{
    _topic = "";
    _chanOps.push_back(chanOpsNick);
}

bool Channel::isChanMember(std::string nick)
{
    for (std::vector<std::string>::iterator it = _members.begin(); it != _members.end(); it++)
    {
        if (*it == nick)
            return true;
    }
    return false;
}

void Channel::addMember(std::string nick)
{
    _members.push_back(nick);
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
    for (std::vector<std::string>::iterator it = _members.begin(); it != _members.end(); it++)
    {
        if (*it == nick)
        {
            _members.erase(it);
            return;
        }
    }
}

void Channel::removeChanOps(std::string nick)
{
    for (std::vector<std::string>::iterator it = _chanOps.begin(); it != _chanOps.end(); it++)
    {
        if (*it == nick)
        {
            _chanOps.erase(it);
            break;
        }
    }
}

void Channel::addChanOps(std::string nick)
{
    _chanOps.push_back(nick);
}

bool Channel::isInviteList(std::string nick)
{
    for (std::vector<std::string>::iterator it = _inviteList.begin(); it != _inviteList.end(); it++)
    {
        if (*it == nick)
            return true;
    }
    return false;
}

void Channel::addInviteList(std::string nick)
{
    _inviteList.push_back(nick);
}

void Channel::removeInviteList(std::string nick)
{
    for (std::vector<std::string>::iterator it = _inviteList.begin(); it != _inviteList.end(); it++)
    {
        if (*it == nick)
        {
            _inviteList.erase(it);
            break;
        }
    }
}