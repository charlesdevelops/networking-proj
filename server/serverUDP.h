#include "server.h"
void *get_in_addr(struct sockaddr *sa);

void *get_in_port(struct sockaddr *sa);

struct addrinfo *setup_UDP(int *sockfd, char *UDP_PORT);

void service_UDP(int sockfd);

int talk_to(char *hostname, char *port, char *payload);