#include "../timetable/timetable.h"
#include <stdbool.h>
#include <netinet/in.h>
typedef struct payload{
  int found; // 0 or 1
  int hops;
  char *routes[MAX_NAME_LENGTH];
  char *address[MAX_PORT + INET6_ADDRSTRLEN + 1]; // their respective address per routes.
  char destination[MAX_NAME_LENGTH];
  char source[MAX_NAME_LENGTH];
  char time[MAX_TIMESTRING];
} PAYLOAD;


