#include "server.h"

#define BACKLOG 10
#define MAXDATASIZE 1024 // max 200 per send.

/**
 * @brief Gets address information for setting up a server.
 *
 * This function fills in the addrinfo structure with the specified port,
 * using IPv4 and passive socket options for TCP connections.
 *
 * @param port A string representing the port number to bind to.
 * @return A pointer to the first addrinfo structure in a linked list of addrinfo structures.
 *         Returns NULL if there is an error in getting the address information.
 */
struct addrinfo *getAddrInfo(char *port);

/**
 * @brief Accepts a new connection on a listening socket.
 *
 * This function waits for a new incoming connection on the specified socket
 * and accepts it, creating a new file descriptor for the connection.
 *
 * @param sockfd The file descriptor of the listening socket.
 * @return The file descriptor for the new connection, or -1 if an error occurs.
 */
int new_connection(int sockfd);

/**
 * @brief Sets up a TCP server socket.
 *
 * This function creates, binds, and listens on a TCP socket with the specified port.
 * It also sets the socket to non-blocking mode and configures it to reuse the address.
 *
 * @param sockfd A pointer to an integer that will hold the file descriptor of the created socket.
 * @param TCP_PORT A string representing the port number to bind the socket to.
 */
void setup_TCP(int *sockfd, char *TCP_PORT);
