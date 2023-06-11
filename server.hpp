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
//#include <sys/select.h>
#include <exception>
#include <vector>

class Server
{
    private:
        std::string pass;
        int sock_fd;
        int port;
        std::vector<pollfd> poll_vc ;
        //bool is_on;
        //char buffer[1024];
        // fd_set read;
        // fd_set write;
        // fd_set reread;
        // fd_set rewrite;
    public:
        Server();
        Server(int port, std::string password);
        ~Server();
        //void launch_socket();
        std::string getPass()const;
        int getSock_fd()const;
        int getPort()const;
        static void signalHandler(int signum);

        // fd_set *getReread();
        // void accept_sock();
        // void receive_sock(int fd);
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