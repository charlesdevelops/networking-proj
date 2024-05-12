#include "server.h"

struct addrinfo *setup_UDP(int *sockfd, char *UDP_PORT);

void service_UDP(int sockfd);

int talk_to(char *hostname, char *port, char *payload);