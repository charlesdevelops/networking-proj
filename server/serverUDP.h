#include "server.h"

struct addrinfo *setup_UDP(int *sockfd, char *UDP_PORT);

void service_UDP(int sockfd);

// talk to using send_to and also free the payload.
int talk_to(char *hostname, char *port, char *payload);