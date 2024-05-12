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

int to_sendfd;
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
      // FD_SET(new_fd, &to_write_fds);
      printf("Received:\n\n%s\n", query);
      response = calloc((strlen(HTTP_200_RESPONSE) + strlen(HTML_200_MESSAGE) + 5), sizeof(char));

      if (response == NULL) perror("calloc");
      sprintf(response, "%s\r\n%s", HTTP_200_RESPONSE,HTML_200_MESSAGE);
      if (send(new_fd, response, strlen(response), 0) == -1) {
          perror("send");
          exit(EXIT_FAILURE);
      };
      to_sendfd = new_fd;
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
      char time[] = "10:00";

      sprintf(payload_tosend, "0 1 1 %s localhost:%s HOME %s %s %s", Station.station_name, UDP_port, time, destination,Station.station_name);
      printf("TcP1\n");
      printf("%s\n", payload_tosend);
      for(int i = 0; i < NUM_NEIGHBOURS; i++){
        talk_to(Neighbours[i].ip_addr, Neighbours[i].udp_port, payload_tosend);
      }
    }
    
    if(FD_ISSET(UDP_fd, &read_fds)){
      printf("UDP1\n");
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
      if(!strcmp(received_payload.destination, Station.station_name)){
        if(received_payload.found == 0){
          received_payload.found = 1;
          int hops = received_payload.hops;

          sprintf(received_payload.address[hops], "%s:%s", "localhost", UDP_port); // unless there's port forwarding, stick with localhost.
          strcpy(received_payload.stations[hops], Station.station_name);
          strcpy(received_payload.routes[hops], Timetable[0].route_name);
          strcpy(received_payload.time[hops], Timetable[0].arrival_time);
          received_payload.hops++;
          received_payload.current = received_payload.hops;
        }

        if(received_payload.found == 2){
          received_payload.found = 3;
          printf("HEOOLLLOO\n");
        }
      }
      
      // payload come back to source
      if(!strcmp(received_payload.source, Station.station_name)){
        if(received_payload.found == 1) {
          printf("\033[0;32m.YAYYY!!!!\n");
          print_payload(received_payload);
          received_payload.found = 2;
        }
        if(received_payload.found == 3) {
          printf("\033[0;32m.HOORAAYY!!!!\n");
          print_payload(received_payload);
          received_payload.found = 4;
        }
      } 
      
      char target_ip[INET6_ADDRSTRLEN], target_port[MAX_PORT];
      
      // if(received_payload.found == 3) {
      //   printf("BACKTRACE 2\n");
      //   get_ip_port(received_payload, target_ip, target_port, --received_payload.current);
      //   char *payload_tosend = craft_payload(received_payload);
      //   printf("payload to send %s\n", payload_tosend);
      //   talk_to(target_ip, target_port, payload_tosend);
      // }
      // if(received_payload.found == 2) {
        
      if (received_payload.found == 4)
      {
        printf("FINISH!");
        char *answer = craft_payload(received_payload);
        send(to_sendfd, answer, strlen(answer), 0);
      } else
      if (received_payload.found == 3)
      {
        get_ip_port(received_payload, target_ip, target_port, --received_payload.current);
        char *payload_tosend = craft_payload(received_payload);
        printf("payload to send %s\n", payload_tosend);
        talk_to(target_ip, target_port, payload_tosend);
      } else
      if(received_payload.found == 2){
        int current = received_payload.current;
        int which = search_timetable(Timetable, NUM_TIMETABLES, received_payload.time[current-1], received_payload.stations[current]);
        printf("HEY");
        strcpy(received_payload.time[current], Timetable[which].arrival_time);
        strcpy(received_payload.routes[current], Timetable[which].route_name);
        printf("Through %s on %s, and arrived at %s on %s", received_payload.routes[current], Timetable[which].departure_time, received_payload.stations[current], Timetable[which].arrival_time);
        get_ip_port(received_payload, target_ip, target_port, ++received_payload.current);
        char *payload_tosend = craft_payload(received_payload);
        printf("payload to send %s\n", payload_tosend);
        talk_to(target_ip, target_port, payload_tosend);
      } else 
      if(received_payload.found == 1){
        // for backtracing.
          // BACKTRACE! definitely in between the routes.
          printf("BACKTRACE!\n");
  
          get_ip_port(received_payload, target_ip, target_port, --received_payload.current);
          char *payload_tosend = craft_payload(received_payload);
          printf("payload to send %s\n", payload_tosend);
          talk_to(target_ip, target_port, payload_tosend);
      } else {
        printf("not found yet\n");
        int hops = received_payload.hops;
        // update the payload.
        sprintf(received_payload.address[hops], "%s:%s", "localhost", UDP_port); // unless there's port forwarding, stick with localhost.
        strcpy(received_payload.stations[hops], Station.station_name);
        strcpy(received_payload.routes[hops], "...");
        strcpy(received_payload.time[hops], "...");
        received_payload.hops++;
        received_payload.current = received_payload.hops;

        // flood neighbours that haven't been reached.
        for(int j = 0; j < NUM_NEIGHBOURS; j++){
          int been_there = 0;
          for(int k = 0; k < hops; k++){
            if(!strcmp(Neighbours[j].addr_and_port, received_payload.address[k])){
              been_there = 1;
              break;
            }
          }
          // the payload's been there, just move on to others.
          if(been_there) continue;
          
          char *payload_tosend = craft_payload(received_payload);
          printf("payload to send: %s\n", payload_tosend);
          talk_to(Neighbours[j].ip_addr, Neighbours[j].udp_port, payload_tosend);
        }
      }
      // print_payload(received_payload);
    }
  }
  return 0;
}
