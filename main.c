/*
 ** server.c -- a stream socket server demo
 */

#include "server/serverTCP.h"
#include "server/serverUDP.h"
#include "server/response_http.h"
#include "timetable/timetable.h"
#include "payload/payload.h"
#include "client/client.h"
#include <time.h>
// #define HTTP_HEADER 60

char TCP_port[MAX_PORT];
char UDP_port[MAX_PORT];

struct query{
  char destination[MAX_NAME_LENGTH];
  char time[MAX_TIMESTRING];
};

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

  time_t now;
  struct tm *now_tm;
  char formatted_time[MAX_TIMESTRING];  // Enough space for "HH:MM"

  // Get current time
  time(&now);
  now_tm = localtime(&now);

  // Format time as HH:MM
  strftime(formatted_time, sizeof(formatted_time), "%H:%M", now_tm);
  printf("Server %s is hosting a TCP socket on port %s and UDP on port %s\n", argv[1], TCP_port, UDP_port);
  printf("Current time is: %s\n", formatted_time);

  NUM_TIMETABLES = readtimetable(timetable_file, &Station, &Timetable);
  NUM_NEIGHBOURS = readneighbours(argc, argv, &Neighbours);

  // print_timetable(Timetable, NUM_TIMETABLES);
  print_neighbours(Neighbours, NUM_NEIGHBOURS);
  int TCP_fd, UDP_fd;
  fd_set read_fds;
  int fdmax; // max fd number
  char query[MAXDATASIZE];
  int nbytes;
  char *response;

  setup_TCP(&TCP_fd, TCP_port);
  struct addrinfo *UDP_addrinfo = setup_UDP(&UDP_fd, UDP_port);
  struct sockaddr_in *addr_in = (struct sockaddr_in *)UDP_addrinfo->ai_addr;
  printf("UDP Port: %d\n", ntohs(addr_in->sin_port));
  fdmax = (UDP_fd > TCP_fd) ? UDP_fd : TCP_fd;
  printf("fdmax: %d\n", fdmax);
  struct timeval tv;
  while(1) {
    FD_ZERO(&read_fds);
    FD_SET(TCP_fd, &read_fds);
    FD_SET(UDP_fd, &read_fds);
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    printf("***\nBack to select\n\n");
    int retval = select(fdmax+1, &read_fds, NULL, NULL, &tv);
    if(retval == -1) {
        perror("select");
        exit(EXIT_FAILURE);
    } else if(retval == 0) {
        // Timeout handling
        CLIENT_NODE *current = head_client; // defined in client.h
        while (current != NULL) {
            if (fcntl(current->fd, F_GETFL) != -1) {  // Check if fd is still open
                if (current->query.answers[0] == '\0') {  // Check if the answer is empty
                    // Resend the query using your talk_to function
                    printf("Resending...\n");
                    char *payload_tosend = (char *)malloc((3 * sizeof(int)) + sizeof(char) * (MAX_NAME_LENGTH + MAX_PORT + INET6_ADDRSTRLEN + 1 + MAX_NAME_LENGTH + MAX_TIMESTRING) + (2 * MAX_NAME_LENGTH));
                    sprintf(payload_tosend, "0 0 0 %s %s %s;", current->query.destination,Station.station_name,current->query.start_time);
                    printf("%s\n", payload_tosend);
                    talk_to("localhost", UDP_port, payload_tosend);  // You will need to adjust this to fit your network communication logic
                }
            } else {
                close(current->fd);
                remove_client(current->fd);  // Remove client from list if fd is not valid
            }
            current = current->next;
        }
        continue;
    }
    if(FD_ISSET(UDP_fd, &read_fds)){
      printf("***\nIn UDP\n\n");
      struct sockaddr_storage client_addr;
      socklen_t addr_len = sizeof(client_addr);
      char *buffer = (char *) malloc(MAXDATASIZE * sizeof(char));
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
      printf("%s\n\n", buffer);
      ///*
    
      PAYLOAD pload;  
      load_payload(&pload, buffer);
      free(buffer);

      // payload arrived at destination.
      if(!strcmp(pload.destination, Station.station_name)){
        if(pload.found == 0){
          pload.found = 1;
          int hops = pload.hops;

          sprintf(pload.address[hops], "%s:%s", "localhost", UDP_port); // unless there's port forwarding, stick with localhost.
          strcpy(pload.stations[hops], Station.station_name);
          strcpy(pload.routes[hops], Timetable[0].route_name);
          strcpy(pload.time[hops], Timetable[0].arrival_time);
          pload.hops++;
          pload.current = pload.hops;
        }

        if(pload.found == 2){
          pload.found = 3;
          printf("Found: 2 => 3\n");
        }
      }

      // payload come back to source
      if(!strcmp(pload.source, Station.station_name)){
        if(pload.found == 1) {
          printf("Found: 1 => 2\n");
          int current = pload.current;
          print_payload(pload);
          printf("Start_Time: %s\n", pload.start_time);
          int which = search_timetable(Timetable, NUM_TIMETABLES, pload.start_time, pload.stations[current]);
          strcpy(pload.time[0], Timetable[which].departure_time); // put the time the transport leaves
          printf("pload.time[0]: %s\n", pload.time[0]);
          print_payload(pload);
          
          pload.found = 2;
        }
        if(pload.found == 3) {
          printf("Found: 3 => 4\n");
          print_payload(pload);
          pload.found = 4;
        }
      } 

      char target_ip[INET6_ADDRSTRLEN], target_port[MAX_PORT];

      if (pload.found == 4)
      {
        printf("FINISH!");
        char *answer = craft_payload(pload);
        // update the answers
        QUERY *q = search_query(pload.destination, pload.start_time);
        if (q->answers[0] == '\0') {
          strcpy(q->answers, answer);
        } else {
          PAYLOAD curr_payload;
          load_payload(&curr_payload, q->answers);
          // update if its faster route (time)
          if(timeToMinutes(curr_payload.time[curr_payload.hops - 1]) > timeToMinutes(pload.time[pload.hops - 1])){
            strcpy(q->answers, answer);
          }
        }
        CLIENT_NODE *fd_tosend;
        while((fd_tosend = find_client(q)) != NULL){
          send(fd_tosend->fd, q->answers, strlen(q->answers), 0);
          close(fd_tosend->fd);
          remove_client(fd_tosend->fd); // client has gotten their answer, remove!
        }
        free(fd_tosend);
      } else
        if (pload.found == 3)
        {
          get_ip_port(pload, target_ip, target_port, --pload.current);
          char *payload_tosend = craft_payload(pload);
          printf("payload to send %s\n", payload_tosend);
          talk_to(target_ip, target_port, payload_tosend);
        } else
          if(pload.found == 2){
            int current = pload.current;
            int which = search_timetable(Timetable, NUM_TIMETABLES, pload.time[current-1], pload.stations[current]);
            strcpy(pload.time[current], Timetable[which].arrival_time);
            strcpy(pload.routes[current], Timetable[which].route_name);
            printf("Through %s on %s, and arrived at %s on %s", pload.routes[current], Timetable[which].departure_time, pload.stations[current], Timetable[which].arrival_time);
            get_ip_port(pload, target_ip, target_port, ++pload.current);
            char *payload_tosend = craft_payload(pload);
            printf("payload to send %s\n", payload_tosend);
            talk_to(target_ip, target_port, payload_tosend);
          } else 
            if(pload.found == 1){
              // for backtracing.
              // BACKTRACE! definitely in between the routes.
              printf("BACKTRACE!\n");

              get_ip_port(pload, target_ip, target_port, --pload.current);
              char *payload_tosend = craft_payload(pload);
              printf("payload to send %s\n", payload_tosend);
              talk_to(target_ip, target_port, payload_tosend);
              free(payload_tosend);
            } else {
              printf("not found yet\n");
              int hops = pload.hops;
              // update the payload.
              sprintf(pload.address[hops], "%s:%s", "localhost", UDP_port); // unless there's port forwarding, stick with localhost.
              strcpy(pload.stations[hops], Station.station_name);
              strcpy(pload.routes[hops], "...");
              strcpy(pload.time[hops], "...");
              pload.hops++;
              pload.current = pload.hops;

              // flood neighbours that haven't been reached.
              for(int j = 0; j < NUM_NEIGHBOURS; j++){
                int been_there = 0;
                for(int k = 0; k < hops; k++){
                  if(!strcmp(Neighbours[j].addr_and_port, pload.address[k])){
                    been_there = 1;
                    break;
                  }
                }
                // the payload's been there, just move on to others.
                if(been_there) continue;

                char *payload_tosend = craft_payload(pload);
                printf("payload to send: %s\n", payload_tosend);
                talk_to(Neighbours[j].ip_addr, Neighbours[j].udp_port, payload_tosend);
              }
            }
      // print_payload(pload);
      ///*/
    }
    if(FD_ISSET(TCP_fd, &read_fds)) {
      // handles client web connection (TCP)
      int new_fd = new_connection(TCP_fd);

      
      bzero(query, MAXDATASIZE);
      if((nbytes = recv(new_fd, query, MAXDATASIZE, 0)) <= 0) {
        if(nbytes == 0) {
          continue;
        } else {
          perror("recv");
        }
      }

      char destination[61];
      char time[MAX_TIMESTRING];
      if (sscanf(query, "GET /?to=%s HTTP/1.1\n", destination) == 1) {
        strcpy(time, formatted_time); // use the current time
        printf("Using time: %s\n", time);
      }
      else if(sscanf(query, "GET /?to=%s&time=%s HTTP/1.1\n", destination, time) == 1) {
        printf("Using time: %s\n", time);
      }
      else {continue;} // not meaningful
      strcpy(time, "10:00");
            // tell browser to wait
      response = calloc(strlen(HTTP_200_RESPONSE) + strlen(HTML_200_MESSAGE) + 5, sizeof(char));

      if (response == NULL) perror("calloc");
      sprintf(response, "%s\r\n%s", HTTP_200_RESPONSE,HTML_200_MESSAGE);

      send(new_fd, response, strlen(response), 0);
      // Stop receiving data from the browser
      if (shutdown(new_fd, SHUT_RD) < 0) {
          perror("Error in shutdown");
          return 1;
      }

      printf("socket %d been accepted and kept to be sent a response \n", new_fd);
      
      // struct timeval timeout;
      // timeout.tv_sec = 1;
      // timeout.tv_usec = 0;
      // setsockopt(TCP_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

      QUERY *q;
      if((q = search_query(destination, time)) == NULL){
        QUERY new_q = new_query();
        strcpy(new_q.start_time, time);
        strcpy(new_q.destination, destination);
        add_query(new_q);
        add_client(new_fd, new_q);
      } else {
        add_client(new_fd, (*q));
      }
      
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
      sprintf(payload_tosend, "0 0 0 %s %s %s;", destination,Station.station_name,time);
      printf("%s\n", payload_tosend);
      // talk_to("localhost", UDP_port, payload_tosend);
      free(payload_tosend);
    }
  }
  return 0;
}
