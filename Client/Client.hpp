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
    std::string _hostname;
    bool _isOperator;
    std::map<std::string, bool> _channels;

public:
    Client() {}
    Client(int fd, std::string pass);
    ~Client() {}

    // setters ----------------
    void setAuth(bool auth) { this->_auth = auth; }
    void setNickname(std::string nickname) { this->_nickname = nickname; }
    void setUsername(std::string username) { this->_username = username; }
    void setOperator(bool isOperator) { this->_isOperator = isOperator; }

    // getters ----------------
    bool getAuth() const { return _auth; }
    int getFd() const { return _fd; }
    std::string getNickname() const { return _nickname; }
    std::string getUsername() const { return _username; }
    std::string getHostname() const { return _hostname; }
    bool getOperator() const { return _isOperator; }
};

#endif