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
    std::vector<int> _chanOps;

public:
    Channel() {}
    Channel(std::string name, std::string key, int chanOps);
    bool clientExist(int client_fd);
    void addClient(int client_fd);

    // Getters -------------------------------------------------------------------
    std::string getName() const { return _name; }
    std::string getTopic() const { return _topic; }
    std::string getKey() const { return _key; }
    std::vector<int> getClients() const { return _clients; }

    // Setters -------------------------------------------------------------------
    void setName(std::string name) { _name = name; }
    void setTopic(std::string topic) { _topic = topic; }
    void setKey(std::string key) { _key = key; }
};

#endif