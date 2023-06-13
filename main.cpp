#include "Server.hpp"

int main(int ac, char **av)
{
  (void)ac;
  int port = atoi(av[1]);
  std::string pass = av[2];
  if (ac != 3)
  {
    std::cerr << "too many arguments\n";
    return (1);
  }
  
  try
  {
    Server my_server(port, pass);
    my_server.start();
  }
  catch (std::exception &e)
  {
    std::cout << e.what() << std::endl;
  }
  return (0);
}
