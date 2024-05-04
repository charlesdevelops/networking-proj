/*
 ** server.c -- a stream socket server demo
 */

#include "serverTCP.h"
#include "serverUDP.h"
#include "response_http.h"

#define TCP_port "4003" // adjustable
#define UDP_port "4004"

int main(void)
{
  int TCP_fd, UDP_fd;

  setup_TCP(&TCP_fd, TCP_port);
  setup_UDP(&UDP_fd, UDP_port);
  if (!fork())
  {
    while (1)
    { // main accept() loop
      // the service_TCP function, need to be broken down?
      service_TCP(TCP_fd);

      /*
      service_TCP has to be broken down, since I'm sending a "trivial" http response whenever there is a connection made to the TCP_PORT.
      make a sendto() to another station server based on the query, this way some modification on service_TCP have to be done.
      */
    }
  }
  else
  {
    while (1)
    {
      // the service_UDP function, need to be broken down?
      service_UDP(UDP_fd);

      /*
      make a choice whether to sendto() to another station server
      (
        need to have struct identity, for it to have char **neighbours, and its own char *name.
        there should be some protocol created to check the inside package, specifically checking on a line (based on the implementation) whether the neighbour has already gotten the packet before.
      )
      OR
      the if UDP packet is the awaited answer to the client webbrowser, then do TCP send() to the browser.
      Hence, there should be some packet inspection that check whether the packet that arrived is the answer to the TCP connection.
      */
    }
  }
  return 0;
}
