#include "Server/Server.hpp"

int g_fd;

void sighandler(int signum)
{
	(void)signum;
	close(g_fd);
	std::cout << "Server terminated\n";
	exit(EXIT_SUCCESS);
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Invalid number of arguments\n";
		return (1);
	}

	int port = atoi(av[1]);
	std::string pass = av[2];

	signal(SIGINT, sighandler);
	Server my_server(port, pass);
	g_fd = my_server.getSock_fd();
	my_server.start();

	return (0);
}

/*
 /part #chan
 whois
 CAP LS
*/