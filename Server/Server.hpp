#ifndef SERVER_HPP
#define SERVER_HPP

#include "../header.h"
#include "../Client/Client.hpp"

class Server
{
    private:
        std::string _pass;
        int _port;
        int _sock_fd;
        sockaddr_in _sockaddr;
        std::vector<pollfd> _poll_vc ;
        std::map<int, Client> _clients;
        Server();

    public:
        Server(int port, std::string password);
        ~Server();
        std::string getPass()const;
        int getSock_fd()const;
        int getPort()const;
        void start();
        void newClient();
        void ClientRecv(int client_fd);

        class Error_Select : public std::exception
        {
            virtual const char *what() const throw();
        };
        class Error_Accept : public std::exception
        {
            virtual const char *what() const throw();
        };
};


#endif