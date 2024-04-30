 #include "server.h"
 
 struct addrinfo *getAddrInfo(char *port){
  int rv;
  struct addrinfo hints, *servers;
  // construct hints
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;
  if ((rv = getaddrinfo(NULL, port, &hints, &servers)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return NULL;
  }
  return servers;
};