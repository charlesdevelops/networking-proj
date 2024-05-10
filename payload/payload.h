#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "../macros/macros.h"
typedef struct payload{
  int found; // 0 or 1
  int hops;
  int current;
  char *routes[MAX_NAME_LENGTH];
  char *address[MAX_PORT + INET6_ADDRSTRLEN + 1]; // their respective address per routes.
  char destination[MAX_NAME_LENGTH];
  char source[MAX_NAME_LENGTH];
  char time[MAX_TIMESTRING];
} PAYLOAD;

char* craft_payload(PAYLOAD payload_struct);
void load_payload(PAYLOAD *payload_struct, char* payload_string);
void print_payload(PAYLOAD payload_struct);