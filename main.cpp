#include "Server/Server.hpp"

int g_break;

void sighandler(int signum)
{
	(void)signum;
	g_break = 0;
}

int main(int ac, char **av)
{
	if (ac != 3)
	{
		std::cerr << "Invalid number of arguments\n";
		return (1);
	}
	g_break = 1;
	int port = atoi(av[1]);
	std::string pass = av[2];

	signal(SIGINT, sighandler);
	Server my_server(port, pass);

	int serverStatus = 0;
	while(g_break && !serverStatus)
		serverStatus = my_server.start();
	return (0);
}
