#include "Client.hpp"

Client::Client()
{
    this->_auth = false;
    this->_nickname = "";
    this->_pass = "";
    this->_username = "";
}

Client::~Client()
{
}

void Client::setAuth(bool auth) { this->_auth = auth; }

void Client::setNickname(std::string nickname) { this->_nickname = nickname; }

void Client::setPass(std::string pass) { this->_pass = pass; }

void Client::setUsername(std::string username) { this->_username = username; }

bool Client::getAuth() const { return (this->_auth); }

std::string Client::getNickname() const { return (this->_nickname); }

std::string Client::getUsername() const { return (this->_username); }
