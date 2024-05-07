/*
 ** server.c -- a stream socket server demo
 */

#include "server/serverTCP.h"
#include "server/serverUDP.h"
#include "server/response_http.h"
#include "timetable/timetable.h"

char TCP_port[MAX_PORT];
char UDP_port[MAX_PORT];

STATION Station;
TIMETABLE *Timetable;
NEIGHBOURS *Neighbours;
int NUM_NEIGHBOURS;
int NUM_TIMETABLES;

int main(int argc, char **argv)
{
  if(argc < 5){
    fprintf(stderr, "Usage: ./<station> <station-name> <TCP> <UDP> <neighbour:UDP> ...\n");
    exit(1);
  }

  char timetable_file[70] = "files/tt-";
  strcat(timetable_file, argv[1]);
  strcpy(TCP_port, argv[2]);
  strcpy(UDP_port, argv[3]);

  NUM_TIMETABLES = readtimetable(timetable_file, &Station, &Timetable);
  NUM_NEIGHBOURS = readneighbours(argc, argv, &Neighbours);

  // print_timetable(Timetable, NUM_TIMETABLES);
  // print_neighbours(Neighbours, NUM_NEIGHBOURS);
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
  struct addrinfo *UDP_addrinfo = setup_UDP(&UDP_fd, UDP_port);
  struct sockaddr_in *addr_in = (struct sockaddr_in *)UDP_addrinfo->ai_addr;
  printf("IPv4 Address: %s\n", inet_ntoa(addr_in->sin_addr));
  printf("UDP Port: %d\n", ntohs(addr_in->sin_port));
  FD_SET(UDP_fd, &master);
  fdmax = (UDP_fd > TCP_fd) ? UDP_fd : TCP_fd;
  printf("fdmax: %d\n", fdmax);
  while(1) {
    read_fds = master;
    if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
      perror("select");
      exit(EXIT_FAILURE);
    }
    if (FD_ISSET(TCP_fd, &read_fds)) {
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
      char payload[MAXDATASIZE];
      strcpy(payload, "");
      sprintf(payload, "%s", destination);
      payload[strlen(destination)] = '\0';
      for(i = 0; i < NUM_NEIGHBOURS; i++){
        talk_to(Neighbours[i].ip_addr, Neighbours[i].udp_port, payload);
      }
    }
    if(FD_ISSET(UDP_fd, &read_fds)){
      struct sockaddr_storage client_addr;
      socklen_t addr_len = sizeof(client_addr);
      char buffer[MAXDATASIZE];
      ssize_t num_bytes = recvfrom(UDP_fd, buffer, MAXDATASIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
      if (num_bytes == -1) {
          perror("recvfrom");
          exit(EXIT_FAILURE);
      }
      // Process received UDP data here
        printf("Received UDP data: %s\n", buffer);
    }
  }
  return 0;
}
