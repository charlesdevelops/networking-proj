#include "server.h"

struct addrinfo *getAddrInfo(char *port);

int new_connection(int sockfd);

void setup_TCP(int *sockfd, char *TCP_PORT);