#include "server.h"
#define BACKLOG 10
#define MAXDATASIZE 1024 // max 200 per send.

struct addrinfo *getAddrInfo(char *port);

int new_connection(int sockfd);

void setup_TCP(int *sockfd, char *TCP_PORT);