#include "server.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
  if ( sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr); // convert to IPv4 sockaddr_in
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void *get_in_port(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_port);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_port);
}
