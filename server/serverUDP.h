#include "server.h"

#define PORT "4950"
#define MAXBUFLEN 100

/**
 * @brief Sets up a UDP server socket.
 *
 * This function creates, binds, and configures a UDP socket for receiving datagrams.
 *
 * @param sockfd A pointer to an integer that will hold the file descriptor of the created socket.
 * @param UDP_PORT A string representing the port number to bind the socket to.
 * @return A pointer to the addrinfo structure containing the socket information.
 *         Returns NULL if there is an error in setting up the UDP socket.
 */
struct addrinfo *setup_UDP(int *sockfd, char *UDP_PORT);

/**
 * @brief Sends data to a specified host using UDP.
 *
 * This function sends the specified payload to the specified hostname and port using UDP.
 *
 * @param hostname The hostname or IP address of the destination.
 * @param port The port number of the destination.
 * @param payload The data to be sent.
 * @return Returns 0 on success, non-zero on failure.
 */
int talk_to(char *hostname, char *port, char *payload);

// void service_UDP(int sockfd);
