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
  fd_set master; // master file descriptor list
  fd_set read_fds;
  int fdmax; // max fd number
  int i, j; // reserved for "for loop"
  char query[MAXDATASIZE];
  char *response;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  setup_TCP(&TCP_fd, TCP_port);
  FD_SET(TCP_fd, &master);
  setup_UDP(&UDP_fd, UDP_port);
  FD_SET(UDP_fd, &master);
  fdmax = TCP_fd;
  while(1) {
    read_fds = master;
    if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
      perror("select");
      exit(4);
    }
  printf("LOOP");
    for(i = 0; i <= fdmax; i++){
      if (FD_ISSET(i, &read_fds)) {
        if (i == TCP_fd) {
          // handles client web connection (TCP)
          int new_fd = new_connection(TCP_fd);
          FD_SET(new_fd, &master);
          if (new_fd > fdmax) {
            fdmax = new_fd;
          }
         
          if(recv(new_fd, query, MAXDATASIZE, 0) <= 0){
            perror("recv");
            exit(1);
          } printf("%s", query);

          char destination[61];
          sscanf(query, "GET /?to=%s HTTP/1.1\n", destination);
        
          response = calloc((strlen(HTTP_200_RESPONSE) + strlen(HTML_200_MESSAGE) + strlen(destination) + 6), sizeof(char));

          if (response == NULL) perror("calloc");
          sprintf(response, "%s\r\n%s\n%s",HTTP_200_RESPONSE,HTML_200_MESSAGE, destination);
          if (send(new_fd, response, strlen(response), 0) == -1) {
              perror("send");
              exit(EXIT_FAILURE);
          }
        } else if(i == UDP_fd) {
          // handles data from stations. (UDP)
          service_UDP(UDP_fd);
        }
        else {
          recv(i, query, MAXDATASIZE, 0);
          printf("Data from connections\n");
        }
      }
    }
  }
  return 0;
}