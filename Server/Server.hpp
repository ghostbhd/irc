#ifndef SERVER_HPP
#define SERVER_HPP

#include "../header.h"
#include "../Client/Client.hpp"

#define WELCOMINGCODE 001
#define RPL_AWAY 381

class Server
{
private:
    std::string _pass;
    int _port;
    int _sock_fd;
    sockaddr_in _sockaddr;
    std::vector<pollfd> _poll_vc;
    std::map<int, Client> _clients;

    std::map<int, std::string> _errorMsg;

    std::string _adminName;
    std::string _adminPass;

    Server();

public:
    // Constructor - Destructor --------------------
    Server(int port, std::string password);
    ~Server();

    // Getters -------------------------------------
    std::string getPass() const;
    int getSock_fd() const;
    int getPort() const;

    // Main functions ------------------------------
    void start();
    void newClient();
    void ClientRecv(int client_fd);

    // Utils ---------------------------------------
    std::string deleteNewLine(char *str);
    std::vector<std::string> splitWithSpaces(std::string str);
    int findClientFdByNick(std::string nick);
    int findClientFdByUser(std::string user);

    // RPLY ----------------------------------------
    void sendWelcomeRpl(int client_fd, int code);

    // Errors --------------------------------------
    void initErrorMsg();
    void sendError(int client_fd, int error_code, std::string command);

    // Commands ------------------------------------
    void mainCommands(int client_fd, std::string cleanLine, std::string cmd);
    void operCmd(int client_fd, std::string cleanLine);
    void privmsg(int client_fd, std::string cleanLine);
};

#endif