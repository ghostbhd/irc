#ifndef SERVER_HPP
#define SERVER_HPP
#include <iostream> //for cout
#include <sys/socket.h> // for socket functions
#include <netinet/in.h> //for sockaddr_in
#include <unistd.h> //for read

class Server
{
    private:
        std::string pass;
        int server_fd;
        int port;
    public:
        Server();
        Server(int port, std::string password);
        ~Server();
        void socket();
        std::string const &getPass()const;
        int const &getServer_fd()const;
        int const &getPort()const;
};


#endif