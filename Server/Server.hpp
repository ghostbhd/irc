#ifndef SERVER_HPP
#define SERVER_HPP

#include "../header.h"
#include "../Client/Client.hpp"
#include "../Channel/Channel.hpp"

#define RPL_WELCOMINGCODE 001
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
    std::vector<pollfd> _poll_vc; // fd, events, revents

    std::map<int, Client> _clients; // fd, Client

    std::map<std::string, Channel> _channels; // channel_name, Channel

    std::map<int, std::string> _errorMsg; // error_code, error_msg

    std::string _adminName;
    std::string _adminPass;

    std::string _hostName;
    Server();

public:
    // Constructor - Destructor --------------------------------------------------
    Server(int port, std::string password);
    ~Server();

    // Getters and Setters -------------------------------------------------------
    std::string getPass() const;
    int getSock_fd() const;
    int getPort() const;

    // Main functions ------------------------------------------------------------
    int start();
    void newClient();
    void ClientRecv(int client_fd);

    // Utils in utils.cpp --------------------------------------------------------
    std::string deleteNewLine(std::string str);
    std::vector<std::string> splitWithChar(std::string str, char c);
    bool isChannelExist(std::string name);
    void closeClient(int client_fd);

    // Client  ________________________________
    int findClientFdByNick(std::string nick);
    int findClientFdByUser(std::string user);

    // Channel ________________________________
    std::string findChannelByFd(int fd);
    bool isChanNameValid(std::string name);
    void MsgToChannel(std::string chanName, std::string msg, int client_fd);

    // RPLY ---------------------------------------------------------------------
    void sendWelcomeRpl(int client_fd, std::string nick);

    // Errors in errors.cpp ------------------------------------------------------
    void initErrorMsg();
    void sendError(int client_fd, int error_code, std::string command);

    // Commands in commands.cpp --------------------------------------------------
    void mainCommands(int client_fd, std::string cleanLine, std::string cmd);
    void operCmd(int client_fd, std::string cleanLine);
    void privmsg(int client_fd, std::string cleanLine);
    // chanOps cmds _______
    void joinCmd(int client_fd, std::string cleanLine);
    void inviteCmd(int client_fd, std::string cleanLine);
    void KickCmd(int client_fd, std::string cleanLine);
    void topicCmd(int client_fd, std::string cleanLine);
    void modeCmd(int client_fd, std::string cleanLine);
    void botCmd(int client_fd, std::string cleanLine);
    void pingCmd(int client_fd, std::string cleanLine);
    void noticeCmd(int client_fd, std::string cleanLine);
    void partCmd(int client_fd, std::string cleanLine);
    void quitCmd(int client_fd, std::string cleanLine);
    void killCmd(int client_fd, std::string cleanLine);
};

#endif