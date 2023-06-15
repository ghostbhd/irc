#include "Server/Server.hpp"

int main(int ac, char **av)
{
  if (ac != 3)
  {
    std::cerr << "too many arguments\n";
    return (1);
  }

  int port = atoi(av[1]);
  std::string pass = av[2];

  Server my_server(port, pass);
  my_server.start();


  return (0);
}
