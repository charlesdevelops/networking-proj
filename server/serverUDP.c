#include "serverUDP.h"
#define PORT "4950"
#define MAXBUFLEN 100

struct addrinfo *setup_UDP(int *sockfd, char *UDP_PORT)
{
  struct addrinfo hints, *p, *servinfo;
  int rv;
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, UDP_PORT, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return NULL;
  }

  for (p = servinfo; p != NULL; p = servinfo->ai_next)
  {
    if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
    {
      perror("socket");
      continue;
    }

    if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1)
    {
      close(*sockfd);
      perror("listener: bind");
      continue;
    }
    break;
  }

  if (p == NULL)
  {
    fprintf(stderr, "failed to bind socket\n");
    return NULL;
  }

  freeaddrinfo(servinfo);
  return p;
}

int talk_to(char *hostname, char *port, char *payload)
{
  int rv;
  int numbytes;
  int sockfd;
  struct addrinfo hints, *servinfo, *p;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // set to AF_INET touse IPv4
  hints.ai_socktype = SOCK_DGRAM;

  if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and make a socket
  for (p = servinfo; p != NULL; p = p->ai_next)
  {
    if ((sockfd = socket(p->ai_family, p->ai_socktype,
                         p->ai_protocol)) == -1)
    {
      perror("talker: socket");
      continue;
    }

    break;
  }
  if (p == NULL)
  {
    fprintf(stderr, "talker: failed to create socket\n");
    return 2;
  }

  if ((numbytes = sendto(sockfd, payload, strlen(payload), 0,
                         p->ai_addr, p->ai_addrlen)) == -1)
  {
    perror("talker: sendto");
    exit(1);
  }

  freeaddrinfo(servinfo);

  printf("talker: sent %d bytes to %s on port %s\n", numbytes, hostname, port);
  close(sockfd);

  return 0;
}
