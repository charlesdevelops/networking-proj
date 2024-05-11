/*
 ** server.c -- a stream socket server demo
 */

#include "server/serverTCP.h"
#include "server/serverUDP.h"
#include "server/response_http.h"
#include "timetable/timetable.h"
#include "payload/payload.h"

#define HTTP_HEADER 60

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
  print_neighbours(Neighbours, NUM_NEIGHBOURS);
  int TCP_fd, UDP_fd;
  fd_set master; // master file descriptor list
  fd_set read_fds;
  fd_set to_write_fds; // this is the descriptor list for the client.
  int fdmax; // max fd number
  char query[HTTP_HEADER];
  int nbytes;
  char *response;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);
  FD_ZERO(&to_write_fds);

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
    FD_ZERO(&read_fds);
    read_fds = master;

    if(select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1){
      perror("select");
      exit(EXIT_FAILURE);
    }
    if(FD_ISSET(TCP_fd, &read_fds)) {
      // handles client web connection (TCP)
      int new_fd = new_connection(TCP_fd);

      if((nbytes = recv(new_fd, query, HTTP_HEADER, 0)) <= 0) {
        
        if(nbytes == 0) {
          continue;
        } else {
          perror("recv");
        }
      }
      printf("socket %d been accepted and kept to be sent a response \n", new_fd);
      FD_SET(new_fd, &to_write_fds);
      printf("Received:\n\n%s\n", query);
      response = calloc((strlen(HTTP_200_RESPONSE) + strlen(HTML_200_MESSAGE) + 5), sizeof(char));

      if (response == NULL) perror("calloc");
      sprintf(response, "%s\r\n%s", HTTP_200_RESPONSE,HTML_200_MESSAGE);
      if (send(new_fd, response, strlen(response), 0) == -1) {
          perror("send");
          exit(EXIT_FAILURE);
      };
      printf("%s", query);

      char destination[61];
      sscanf(query, "GET /?to=%s HTTP/1.1\n", destination);

 /*
 The crafting of the first payload here.
  int found; // 0 or 1
  int hops;
  int current;
  char **stations;
  char **address; // their respective address per routes.
  char **routes;
  char **time; // their times.
  char destination[MAX_NAME_LENGTH];
  char source[MAX_NAME_LENGTH];
*/
      char *payload_tosend = (char *)malloc((3 * sizeof(int)) + sizeof(char) * (MAX_NAME_LENGTH + MAX_PORT + INET6_ADDRSTRLEN + 1 + MAX_NAME_LENGTH + MAX_TIMESTRING) + (2 * MAX_NAME_LENGTH));
      PAYLOAD p; // payload

      // INITIALIZE the PAYLOAD contents.
      // char *address[MAX_PORT + INET6_ADDRSTRLEN + 1];
      // address[0] = malloc((MAX_PORT + INET6_ADDRSTRLEN + 1) * sizeof(char));
      // char *routes[MAX_NAME_LENGTH]; 
      // routes[0] = malloc(MAX_NAME_LENGTH * sizeof(char));
      // strcpy(routes[0], Station.station_name);
      // char time[] = "99:99"; // what time does it start.
      // sprintf(address[0], "localhost:%s", UDP_port);

      // // PUT IN PAYLOAD
      // p.found = 0; p.hops = 1; p.current = 1; // not found, first hop, on first station (using 1-indexing);
      // strcpy(p.time, time);
      // strcpy(p.source, Station.station_name);
      // strcpy(p.destination, destination);
      // memcpy(p.routes, routes, MAX_NAME_LENGTH * sizeof(char));
      // memcpy(p.address, address, (MAX_PORT + INET6_ADDRSTRLEN + 1) * sizeof(char));
      // char* payload_tosend = craft_payload(p);
      // printf("The payload: %s\n", payload_tosend);
      char time[] = "10:00";

      sprintf(payload_tosend, "0 1 1 %s localhost:%s HOME %s %s %s", Station.station_name, UDP_port, time, destination,Station.station_name);
      printf("TcP1\n");
      printf("%s\n", payload_tosend);
      for(int i = 0; i < NUM_NEIGHBOURS; i++){
        talk_to(Neighbours[i].ip_addr, Neighbours[i].udp_port, payload_tosend);
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

      /*
        The crafting of the intermediate payload is here.
      */
      // Process received UDP data here
      buffer[num_bytes] = '\0'; // IMPORTANT

      PAYLOAD received_payload;  
      load_payload(&received_payload, buffer);

      // payload arrived at destination.
      if(!strcmp(received_payload.destination, Station.station_name) && received_payload.found == 0){
        received_payload.found = 1;
        int hops = received_payload.hops;
        // received_payload.address[hops] = malloc(MAX_PORT + INET6_ADDRSTRLEN + 1);
        // received_payload.routes[hops] = malloc(MAX_NAME_LENGTH);
        sprintf(received_payload.address[hops], "%s:%s", "localhost", UDP_port); // unless there's port forwarding, stick with localhost.
        strcpy(received_payload.stations[hops], Station.station_name);
        strcpy(received_payload.routes[hops], Timetable[0].route_name);
        strcpy(received_payload.time[hops], Timetable[0].arrival_time);
        received_payload.hops++;
        received_payload.current = received_payload.hops;
      }

      // payload arrived at destination 2nd time.

      
      if(received_payload.found){
        if(!strcmp(received_payload.source, Station.station_name)){
          /*
            Time to get the routes, and timeframes.
          */
         
        } else {
          // BACKTRACE! definitely a neighbour.
          printf("BACKTRACE!\n");
          char ip_target[INET6_ADDRSTRLEN];
          char port_target[MAX_PORT];
          int current =  --received_payload.current;
          sscanf(received_payload.address[current-1], "%45[^:]:%s", ip_target, port_target);
          printf("%s\n", received_payload.address[current-1]);
          char *payload_tosend = craft_payload(received_payload);
          printf("payload to send %s\n", payload_tosend);
          talk_to(ip_target, port_target, payload_tosend);
          }
      }
      else {
        printf("not found yet\n");
        int hops = received_payload.hops;
        // received_payload.address[hops] = malloc(MAX_PORT + INET6_ADDRSTRLEN + 1);
        // received_payload.routes[hops] = malloc(MAX_NAME_LENGTH);
        sprintf(received_payload.address[hops], "%s:%s", "localhost", UDP_port); // unless there's port forwarding, stick with localhost.
        strcpy(received_payload.stations[hops], Station.station_name);
        strcpy(received_payload.routes[hops], "...");
        strcpy(received_payload.time[hops], "...");
        received_payload.hops++;
        received_payload.current = received_payload.hops;

        for(int j = 0; j < NUM_NEIGHBOURS; j++){
          int been_there = 0;
          for(int k = 0; k < hops; k++){
            if(!strcmp(Neighbours[j].addr_and_port, received_payload.address[k])){
              been_there = 1;
              break;
            }
          }
          // the data's been there, just move on to others.
          if(been_there) continue;
          printf("This is the station's timetable\n");
          print_timetable(Timetable, NUM_TIMETABLES);
          
          char *payload_tosend = craft_payload(received_payload);
          talk_to(Neighbours[j].ip_addr, Neighbours[j].udp_port, payload_tosend);
        }
      }
      print_payload(received_payload);
    }
  }
  return 0;
}
