#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../header.h"

class Channel
{
private:
    std::string _name;
    std::vector<std::string> _clients;
    std::string _topic;
    std::string _key;
    std::vector<std::string> _chanOps;
    bool _inviteOnly;
    //bool _limit; // TODO: implement limit

public:
    Channel() {}
    Channel(std::string name, std::string key, std::string chanOpsNick);

    // Utils ---------------------------------------------------------------------

    bool isChanMember(std::string nick);
    void addClient(std::string nick);
    bool isChanOps(std::string nick);
    void removeClient(std::string nick);

    // Getters -------------------------------------------------------------------
    std::string getName() const { return _name; }
    std::string getTopic() const { return _topic; }
    std::string getKey() const { return _key; }
    std::vector<std::string> getClients() const { return _clients; }
    bool getInviteOnly() const { return _inviteOnly; }

    // Setters -------------------------------------------------------------------
    void setName(std::string name) { _name = name; }
    void setTopic(std::string topic) { _topic = topic; }
    void setKey(std::string key) { _key = key; }
    void setInviteOnly(bool inviteOnly) { _inviteOnly = inviteOnly; }
};

#endif