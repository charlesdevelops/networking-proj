#include "serverUDP.h"
#define PORT "4950"
#define MAXBUFLEN 100

struct addrinfo *setup_UDP(int *sockfd, char *UDP_PORT){
  struct addrinfo hints, *p, *servinfo;
  int rv;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if((rv = getaddrinfo(NULL, UDP_PORT, &hints, &servinfo)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return NULL;
  }

  for(p = servinfo; p != NULL; p = servinfo->ai_next){
    if((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1){
      perror("socket");
      continue;
    }

    if(bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(*sockfd);
      perror("listener: bind");
      continue;
    }
    break;
  }

  if(p == NULL) {
    fprintf(stderr, "failed to bind socket\n");
    return NULL;
  }
  // Set non-blocking flag
  // int flags = fcntl(*sockfd, F_GETFL, 0);
  // if (flags == -1) {
  //   perror("fcntl get");
  //   exit(EXIT_FAILURE);
  // }

  // if (fcntl(*sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
  //   perror("fcntl set");
  //   exit(EXIT_FAILURE);
  // }

  freeaddrinfo(servinfo);
  return p;
}

// void service_UDP(int sockfd){
//   int numbytes;
//   struct sockaddr_storage client_addr;
//   socklen_t addr_len;
//   char msg[MAXBUFLEN];
//   char s[INET6_ADDRSTRLEN];

//    addr_len = sizeof client_addr;
//   if ((numbytes = recvfrom(sockfd, msg, MAXBUFLEN-1, 0, (struct sockaddr*)&client_addr, &addr_len)) == -1){
//       perror("recvfrom");
//       exit(1);
//     }
  
//     printf("listener: got packet from %s port %hu\n", inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *)&client_addr), s, sizeof s), *(in_port_t *)get_in_port((struct sockaddr *)&client_addr));

//     printf("listener: packet is %d bytes long\n", numbytes);
//     msg[numbytes] = '\0';
//     printf("listener: packet contains \"%s\"\n", msg);
//     if (!strcmp(msg, "END")) return;
// }

int talk_to(char *hostname, char *port, char *payload){
  int rv;
  int numbytes;
  int sockfd;
  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // set to AF_INET touse IPv4
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
  }

  // loop through all the results and make a socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
              p->ai_protocol)) == -1) {
          perror("talker: socket");
          continue;
      }

      break;
  }
  if (p == NULL) {
        fprintf(stderr, "talker: failed to create socket\n");
        return 2;
  }

  if ((numbytes = sendto(sockfd, payload, strlen(payload), 0,
           p->ai_addr, p->ai_addrlen)) == -1) {
      perror("talker: sendto");
      exit(1);
  }

  freeaddrinfo(servinfo);

  printf("talker: sent %d bytes to %s on port %s\n", numbytes, hostname, port);
  close(sockfd);

  return 0;
}
