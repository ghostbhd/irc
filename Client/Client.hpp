#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../header.h"

class Client
{
private:
    int _fd;
    bool _auth;
    std::string _nickname;
    std::string _pass;
    std::string _username;
    Client() {}

public:
    Client(int fd, std::string pass);
    ~Client() {}

    // setters ----------------
    void setAuth(bool auth) { this->_auth = auth; }
    void setNickname(std::string nickname) { this->_nickname = nickname; }
    void setUsername(std::string username) { this->_username = username; }

    // getters ----------------
    bool getAuth() const { return _auth; }
    int getFd() const { return _fd; }
    std::string getNickname() const { return _nickname; }
    std::string getUsername() const { return _username; }
};

#endif