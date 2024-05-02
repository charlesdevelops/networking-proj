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
  if(!fork()){
    while(1) { // main accept() loop
      service_TCP(TCP_fd);
    }
  } else {
    while(1) {
      service_UDP(UDP_fd);
    }
  }
  return 0;
}
