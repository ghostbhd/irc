#ifndef SERVER_HPP
#define SERVER_HPP

#include "../header.h"
#include "../Client/Client.hpp"
#include "../Channel/Channel.hpp"

#define WELCOMINGCODE 001
#define RPL_AWAY 381
#define RPL_INVITING 341
#define RPL_TOPIC 332

class Server
{
private:
    std::string _pass;
    int _port;
    int _sock_fd;
    sockaddr_in _sockaddr;
    std::vector<pollfd> _poll_vc;

    std::map<int, Client> _clients;

    std::map<std::string, Channel> _channels;

    std::map<int, std::string> _errorMsg;

    std::string _adminName;
    std::string _adminPass;

    Server();

public:
    // Constructor - Destructor --------------------------------------------------
    Server(int port, std::string password);
    ~Server();

    // Getters -------------------------------------------------------------------
    std::string getPass() const;
    int getSock_fd() const;
    int getPort() const;

    // Main functions ------------------------------------------------------------
    void start();
    void newClient();
    void ClientRecv(int client_fd);

    // Utils ---------------------------------------------------------------------
    std::string deleteNewLine(std::string str);
    std::vector<std::string> splitWithChar(std::string str, char c);
    bool isChannelExist(std::string name);

    // Client ********************************
    int findClientFdByNick(std::string nick);
    int findClientFdByUser(std::string user);

    // Channel *******************************
    std::string findChannelByFd(int fd);
    bool isChanNameValid(std::string name);
    void MsgToChannel(std::string chanName, std::string msg, int client_fd);

    // RPLY ---------------------------------------------------------------------
    void sendWelcomeRpl(int client_fd, std::string nick, int code, std::string param);

    // Errors -------------------------------------------------------------------
    void initErrorMsg();
    void sendError(int client_fd, int error_code, std::string command);

    // Commands -----------------------------------------------------------------
    void mainCommands(int client_fd, std::string cleanLine, std::string cmd);
    void operCmd(int client_fd, std::string cleanLine);
    void privmsg(int client_fd, std::string cleanLine);
    void joinCmd(int client_fd, std::string cleanLine);
    void inviteCmd(int client_fd, std::string cleanLine);
    void KickCmd(int client_fd, std::string cleanLine);
    void topicCmd(int client_fd, std::string cleanLine);
    void modeCmd(int client_fd, std::string cleanLine);
};

#endif