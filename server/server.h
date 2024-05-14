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

/**
 * @brief Retrieves the address (IPv4 or IPv6) from a sockaddr structure.
 *
 * This function extracts the address from a given sockaddr structure,
 * which can be either of type IPv4 (sockaddr_in) or IPv6 (sockaddr_in6).
 *
 * @param sa A pointer to a sockaddr structure.
 * @return A pointer to the address part of the sockaddr structure.
 */
void *get_in_addr(struct sockaddr *sa);

/**
 * @brief Retrieves the port number from a sockaddr structure.
 *
 * This function extracts the port number from a given sockaddr structure,
 * which can be either of type IPv4 (sockaddr_in) or IPv6 (sockaddr_in6).
 *
 * @param sa A pointer to a sockaddr structure.
 * @return A pointer to the port number part of the sockaddr structure.
 */
void *get_in_port(struct sockaddr *sa);

#endif
