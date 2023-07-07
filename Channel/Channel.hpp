#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "../header.h"

class Channel
{
private:
    std::string _name;
    std::vector<std::string> _members;
    std::string _topic;
    std::string _key;
    std::vector<std::string> _chanOps;
    std::vector<std::string> _inviteList;
    bool _inviteOnly;
    bool _topicRestriction;
    bool _limit; // TODO: implement limit
    int _limitValue;

public:
    Channel() {}
    Channel(std::string name, std::string key, std::string chanOpsNick);

    // Utils ---------------------------------------------------------------------

    bool isChanMember(std::string nick);

    void addMember(std::string nick);
    void removeMember(std::string nick);

    bool isChanOps(std::string nick);
    void addChanOps(std::string nick);
    void removeChanOps(std::string nick);

    bool isInviteList(std::string nick);
    void addInviteList(std::string nick);
    void removeInviteList(std::string nick);


    // Getters -------------------------------------------------------------------
    std::string getName() const { return _name; }
    std::string getTopic() const { return _topic; }
    std::string getKey() const { return _key; }
    std::vector<std::string> getMembers() const { return _members; }
    bool getInviteOnly() const { return _inviteOnly; }
    bool getTopicRestriction() const { return _topicRestriction; }
    bool getLimit() const { return _limit; }
    int getLimitValue() const { return _limitValue; }

    // Setters -------------------------------------------------------------------
    void setName(std::string name) { _name = name; }
    void setTopic(std::string topic) { _topic = topic; }
    void setKey(std::string key) { _key = key; }
    void setInviteOnly(bool inviteOnly) { _inviteOnly = inviteOnly; }
    void setTopicRestriction(bool rest) { _topicRestriction = rest; }
    void setLimit(int limit) { _limit = limit; }
    void setLimitValue(int limitValue) { _limitValue = limitValue; }
};

#endif