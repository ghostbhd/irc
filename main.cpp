#include "server.hpp"
//#include "Client.hpp"

// void signalHandler(int signum)
// {
//   exit(signum);
// }

int main (int ac, char **av)
{
  (void)ac;
  int port = atoi(av[1]);
  std::string pass = av[2];
  if (ac != 3)
  {
    std::cerr << "too many arguments\n";
    return (1);
  }
  //signal(SIGABRT, signalHandler);
  try
  {
    Server my_server = Server(port, pass);
  }
  catch (std::exception &e)
  {
    std::cout << e.what() << std::endl;
  }
  return (0);
}


// int main(int ac, char **av)
// {
//   int port = atoi(av[1]);
//   std::string pass = av[2];
//   if (ac != 3)
//   {
//     std::cerr << "Too many arguments!\n";
//     return (1);
//   }
//   while (1)
//   {
//     try
//     {
//       Server my_server = Server(port, pass);
//     }
//     catch(const std::exception& e)
//     {
//       std::cerr << e.what() << '\n';
//     }
//     return (0);
//   }

// }