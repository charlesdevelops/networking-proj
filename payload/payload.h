#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "../macros/macros.h"

typedef struct payload{
  int found; // 0 or 1 or 2 or 3 or 4.
  int hops;
  int current;
  char **stations;
  char **address; // their respective address per routes.
  char **routes;
  char **time; // their times.
  char destination[MAX_NAME_LENGTH];
  char source[MAX_NAME_LENGTH];
  char start_time[MAX_TIMESTRING];
} PAYLOAD;


char* craft_payload(PAYLOAD payload_struct);
void load_payload(PAYLOAD *payload_struct, char* payload_string);
void print_payload(PAYLOAD payload_struct);
void get_ip_port(PAYLOAD payload_struct, char* ip, char* port, int which);