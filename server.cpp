#include "server.hpp"

Server::Server()
{
    std::cout << "Launching The Server" << std::endl;
}

void Server::signalHandler(int signum)
{
  exit(signum);
}


Server::Server(int port, std::string password)
{
    this->pass = password;
    this->port = port;

    int connection;
    int len;
    char buff[100];
    
    this->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sock_fd < 0)
    {
        std::cerr << "Cannot create socket!\n";
        exit(EXIT_FAILURE);
    }

    sockaddr_in sockaddr;
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = INADDR_ANY;
    sockaddr.sin_port = htons(this->port);

    if (bind(this->sock_fd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) //struct used to specify the @ assigned to the sock
    {
        std::cerr << "Server cannot bind to the port\n";
        exit (EXIT_FAILURE);
    }
    if (listen(this->sock_fd, 100)) //marks a socket as passive, holds at most 100 connections
    {
        std::cerr << "Server cannot listen on socket\n";
        exit (EXIT_FAILURE);
    }

    len = sizeof(sockaddr);
    connection = accept(this->sock_fd, (struct sockaddr*)&sockaddr, (socklen_t*)&len);
    if (connection < 0)
    {
        std::cerr << "Server cannot connect\n";
        exit (EXIT_FAILURE);
    }
    std::cout << "Server launched !\n";
    signal(SIGABRT, signalHandler);
    while (1)
    {
        read(connection, buff, 100);
        std::cout << buff;
        recv(this->sock_fd, buff, 100, 0);
    }
    close(connection);
    close(this->sock_fd);

}

// Server::Server(int port, std::string password)
// {
//     //char buff[1024]; 
//     this->pass = password;
//     this->port = port;

//     ///********Launching Sockets*********////////
//     int opt = 1;
//     sockaddr_in sockaddr;
//     sockaddr.sin_family = AF_INET; //address family ==> IPv4
//     sockaddr.sin_addr.s_addr = INADDR_ANY; //from any address
//     sockaddr.sin_port = htons(this->port); //convert a number to network byte order

//     std::memset(&sockaddr, 0, sizeof sockaddr);

//     this->sock_fd = socket(AF_INET, SOCK_STREAM, 0); //SOCK_STREAM => create a TCP socket
//     if (this->sock_fd < 0)
//     {
//         std::cerr << "Cannot create socket!\n";
//         exit(EXIT_FAILURE);
//     }
//     if (setsockopt(this->sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) //reuse a local address and port number that is already in use by another socket
//     {
//         std::cerr << "Server cannot reuse socket\n";
//         exit(EXIT_FAILURE);
//     }
//     if (fcntl(this->sock_fd, F_SETFL, O_NONBLOCK) < 0) //changes the properties of the file corresponding to fd
//     {
//         std::cerr << "Blocking socket\n";
//         exit(EXIT_FAILURE); 
//     }
//     if (bind(this->sock_fd, (struct sockaddr*) &sockaddr, sizeof(sockaddr)) < 0) //struct used to specify the @ assigned to the sock
//     {
//         std::cerr << "Server cannot bind to the port\n";
//         exit (EXIT_FAILURE);
//     }
//     if (listen(this->sock_fd, 100)) //marks a socket as passive, holds at most 100 connections
//     {
//         std::cerr << "Server cannot listen on socket\n";
//         exit (EXIT_FAILURE);
//     }


//     ///********Handling Multiplexing*********////////
//     pollfd serv_poll;
//     std::memset(&serv_poll, 0, sizeof(serv_poll));

//     struct sockaddr_in client_addr;
//     socklen_t len = sizeof(client_addr);
//     struct pollfd client_poll;
//     int client_fd;

//     serv_poll.fd = this->sock_fd;
//     serv_poll.events = POLL_IN; //for read events
//     this->poll_vc.push_back(serv_poll);

//     while (1)
//     {
//         if (poll(poll_vc.data(), this->poll_vc.size(), 0) < 0)
//         {
//             std::cerr << "Server Cannot execute multiple clients\n";
//             exit(EXIT_FAILURE);
//         }
//         for (unsigned int i=0; i < poll_vc.size(); i++)
//         {
//             pollfd &instant_client = poll_vc[i];
//             if (!(instant_client.revents &POLLIN))
//                 continue ;
//             if (instant_client.fd == this->sock_fd)
//             {
//                 try
//                 {
//                     client_fd = accept(this->sock_fd, (struct sockaddr*)&client_addr, &len);
//                     fcntl(client_fd, F_SETFL, O_NONBLOCK);
//                     if (client_fd < 0)
//                     {
//                         std::cerr << "Cannot accept client !\n";
//                         exit(EXIT_FAILURE);
//                     }
//                     client_poll.fd = client_fd;
//                     client_poll.events = POLLIN;
//                     //client = 

//                 }
//                 catch(const std::exception& e)
//                 {
//                     std::cerr << e.what() << '\n';
//                 }
                
//             }
//         }
//     }

// }



// void Server::launch_socket(void)
// {
//     int slqt;
//     struct timeval timeout;
//     fd_set reread;
//     fd_set rewrite;

//     while (1)
//     {
//         timeout.tv_sec = 3;
//         timeout.tv_usec = 300;
//         reread = this->read;
//         rewrite = this->write;
//         slqt = select(this->sock_fd + 1, &reread, &rewrite, NULL, &timeout);
//         if (slqt < 0)
//             throw (Server::Error_Select());
//         else if (slqt == 0)
//             continue;
//         else
//             break;
//     }
// }

// fd_set *Server::getReread()
// {
//     return &(this->reread);
// }

// void Server::accept_sock(void)
// {
//     int client_fd = 0;
//     struct sockaddr clientaddr;
//     unsigned int len = sizeof(clientaddr);

//     client_fd = accept(this->sock_fd, (struct sockaddr*)&clientaddr, &len);
//     if (client_fd < 0)
//         throw(Server::Error_Accept());
//     fcntl(client_fd, F_SETFL, O_NONBLOCK);
//     FD_SET(client_fd, &read);
//     FD_SET(client_fd, &write);

//     //create and allocate new clients, client class.
// }

// Server::Server(int port, std::string pass)
// {
//     (void)port;
//     (void)pass;
//     //this->launch_socket();
//     int slqt;
//     struct timeval timeout;

//     while (1)
//     {
//         timeout.tv_sec = 3;
//         timeout.tv_usec = 300;
//         this->reread = this->read;
//         this->rewrite = this->write;
//         slqt = select(this->sock_fd + 1, &reread, &rewrite, NULL, &timeout);
//         if (slqt < 0)
//             throw (Server::Error_Select());
//         else if (slqt == 0)
//             continue;
//         else
//             break;
//     }

//     for (int i = 0; i < 1000; i++) // 1000 Max file descriptors
//     {
//         if (FD_ISSET(i, getReread()))
//         {
//             this->sock_fd = i;
//             if (i == getSock_fd())
//             {
//                 this->accept_sock();
//                 std::cout << "Sockets ACCEPTED !\n";
//             }
//             else
//             {
//                 receive_sock(this->sock_fd);
//             }
//         }
//     }
// }

Server::~Server()
{
    std::cout << "Server is OFF !\n";
}

int Server::getPort()const
{
    return (this->port);
}

int Server::getSock_fd()const
{
    return (this->sock_fd);
}

std::string Server::getPass()const
{
    return (this->pass);
}

const char *Server::Error_Select::what() const throw()
{
    return ("Error: Cannot SELECT socket!\n");
}

const char *Server::Error_Accept::what() const throw()
{
    return ("Error: Cannot Accept socket!\n");
}