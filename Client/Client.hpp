#ifndef CLIENT_HPP
#define CLIENT_HPP


#include "../header.h"

class Client
{
private:
    bool _auth;
    std::string _nickname;
    std::string _pass;
    std::string _username;

public:
    Client();
    ~Client();
    void setAuth(bool auth);
    void setNickname(std::string nickname);
    void setPass(std::string pass);
    void setUsername(std::string username);
    bool getAuth()const;
    std::string getNickname()const;
    std::string getUsername()const;

};

#endif