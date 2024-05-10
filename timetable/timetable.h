#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "../macros/macros.h"
/*
# station-name,longitude,latitude
StationC,115.8300,-31.7600
# departure-time,route-name,departing-from,arrival-time5,arrival-station
06:03,busC_B,stopC,06:23,BusportB
*/

typedef struct station {
  char station_name[MAX_NAME_LENGTH];
  char longitude[MAX_COORDINATE];
  char latitude[MAX_COORDINATE];
}STATION;

typedef struct neighbours {
  char ip_addr[INET6_ADDRSTRLEN];
  char udp_port[MAX_PORT];
  char addr_and_port[INET6_ADDRSTRLEN + MAX_PORT + 1];
}NEIGHBOURS;

typedef struct Timetable{
  char departure_time[MAX_TIMESTRING];
  char route_name[MAX_NAME_LENGTH];
  char departing_from[MAX_NAME_LENGTH];
  char arrival_time[MAX_TIMESTRING];
  char arrival_station[MAX_NAME_LENGTH];
} TIMETABLE;

// return number of schedules.
int readtimetable(char *timetable, STATION *Station, TIMETABLE **Station_Timetable);

// return number of neighbours
int readneighbours(int argc, char **argv, NEIGHBOURS **neighbours);
void print_timetable(TIMETABLE *timetable, int NUM_TIMETABLES);
void print_neighbours(NEIGHBOURS *neighbours, int NUM_NEIGHBOURS);