#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../header.h"

class Client
{
private:
    int _fd;
    bool _auth;
    std::string _pass;
    std::string _nickname;
    std::string _username;
    bool _isOperator;
    std::map<std::string, bool> _channels;
    bool _registered;

public:
    Client() {}
    Client(int fd, std::string pass);
    ~Client() {}

    // setters ----------------
    void setAuth(bool auth) { this->_auth = auth; }
    void setNickname(std::string nickname) { this->_nickname = nickname; }
    void setUsername(std::string username) { this->_username = username; }
    void setOperator(bool isOperator) { this->_isOperator = isOperator; }
    void setRegistered(bool registered) { this->_registered = registered; }

    // getters ----------------
    bool getAuth() const { return _auth; }
    int getFd() const { return _fd; }
    std::string getNickname() const { return _nickname; }
    std::string getUsername() const { return _username; }
    bool getOperator() const { return _isOperator; }
    bool getRegistered() const { return _registered; }
};

#endif