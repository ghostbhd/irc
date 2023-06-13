#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream> //for cout
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> //for sockaddr_in
#include <unistd.h> //for read
#include <fcntl.h> //for non_blocking sockets
#include <sys/poll.h> //for multiplexing
#include <cstdlib> //for exit() and EXIT_FAILURE
#include <csignal> //for Cntrl+c / +z
#include <exception>
#include <cstring> //for memset
#include <vector>

class Server
{
    private:
        std::string _pass;
        int _port;
        int _sock_fd;
        sockaddr_in _sockaddr;
        std::vector<pollfd> _poll_vc ;
        //char buffer[500];
        Server();

    public:
        Server(int port, std::string password);
        ~Server();
        std::string getPass()const;
        int getSock_fd()const;
        int getPort()const;
        void start();

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