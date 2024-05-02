#ifndef _CITS3002_SERVER
#define _CITS3002_SERVER 1
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#endif

#define MAXDATASIZE 200 // max 200 per send.

struct addrinfo *getAddrInfo(char *port);

void *get_in_addr(struct sockaddr *sa);

void service_TCP(int sockfd);

void setup_TCP(int *sockfd, char *TCP_PORT);