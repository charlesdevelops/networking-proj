#ifndef PAYLOAD_H
#define PAYLOAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "../macros/macros.h"

// Structure to represent a payload used in network communication
typedef struct payload
{
  int found;                         // Status indicator, values can be 0, 1, 2, 3, or 4
  int hops;                          // Number of hops/stations the payload has traversed
  int current;                       // Current station index
  char **stations;                   // Array of station names
  char **address;                    // Array of addresses corresponding to each station
  char **routes;                     // Array of routes between stations
  char **time;                       // Array of time strings for each station/route
  char destination[MAX_NAME_LENGTH]; // Destination station name
  char source[MAX_NAME_LENGTH];      // Source station name
  char start_time[MAX_TIMESTRING];   // Start time of the journey
} PAYLOAD;

/**
 * @brief Crafts a payload string from a PAYLOAD structure.
 *
 * This function takes a PAYLOAD structure and constructs a string representation
 * of it, which can be used for network transmission. The resulting string is
 * dynamically allocated and should be freed by the caller.
 *
 * @param payload_struct The PAYLOAD structure to convert.
 * @return A dynamically allocated string containing the payload data.
 */
char *craft_payload(PAYLOAD payload_struct);

/**
 * @brief Loads a payload string into a PAYLOAD structure.
 *
 * This function parses a given payload string and populates the fields of a
 * PAYLOAD structure accordingly. Memory for the string arrays within the
 * structure is dynamically allocated based on the content of the payload string.
 *
 * @param payload_struct Pointer to the PAYLOAD structure to populate.
 * @param payload_string The payload string to parse.
 */
void load_payload(PAYLOAD *payload_struct, char *payload_string);

/**
 * @brief Prints the contents of a PAYLOAD structure.
 *
 * This function outputs the contents of a PAYLOAD structure to the standard output
 * in a readable format. It displays all the fields, including the arrays of
 * stations, addresses, routes, and times.
 *
 * @param payload_struct The PAYLOAD structure to print.
 */
void print_payload(PAYLOAD payload_struct);

/**
 * @brief Extracts the IP address and port from a PAYLOAD structure.
 *
 * This function takes a PAYLOAD structure and extracts the IP address and port
 * from the specified station's address. The extracted IP and port are copied
 * into the provided buffers.
 *
 * @param payload_struct The PAYLOAD structure containing the address information.
 * @param ip Buffer to store the extracted IP address.
 * @param port Buffer to store the extracted port.
 * @param which The index of the station whose IP and port are to be extracted.
 */
void get_ip_port(PAYLOAD payload_struct, char *ip, char *port, int which);

#endif // PAYLOAD_H
