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
#include <fcntl.h>
#include "../macros/macros.h"
#endif

void *get_in_addr(struct sockaddr *sa);
void *get_in_port(struct sockaddr *sa);