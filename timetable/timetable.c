#include "timetable.h"

int readtimetable(char *timetable, STATION *Station, TIMETABLE **Station_Timetable){
  FILE *file = fopen(timetable, "r");
  if (file == NULL) {
    perror("Error opening file");
    return 1;
  }

  int count = 0;
  int size = 1;
  *Station_Timetable = (TIMETABLE *) malloc(size * sizeof(TIMETABLE));
  if(*Station_Timetable == NULL) {
    perror("malloc");
    exit(1);
  }

  char line[MAX_BUFFER];
  while(fgets(line, sizeof line, file)) {
    if(line[0] == '#') {
      continue;
    }

    if(sscanf(line, 
    "%60[^,],%10[^,],%10[^\n]\n"
    , 
    Station->station_name,
    Station->longitude,
    Station->latitude) == 3) {
      break;
    }
  }

  
  while (fgets(line, sizeof line, file)) {
    if(line[0] == '#') {
      continue;
    }

    if(sscanf(line, "%5[^,],%60[^,],%60[^,],%5[^,],%60[^\n]\n", 
        (*Station_Timetable)[count].departure_time,
        (*Station_Timetable)[count].route_name,
        (*Station_Timetable)[count].departing_from,
        (*Station_Timetable)[count].arrival_time,
        (*Station_Timetable)[count].arrival_station) == 5) {
    count++;
      if(count == size) {
        size *= 2;
        TIMETABLE *temp = (TIMETABLE *) realloc(*Station_Timetable, size * sizeof(TIMETABLE));
        if(temp == NULL) {
          fprintf(stderr, "Memory realloc()ation failed.\n");
          exit(1);
        }
        (*Station_Timetable) = temp;
      }
    }
  }
  fclose(file);

  return count;
}

int readneighbours(int argc, char **argv, NEIGHBOURS **neighbours){
  int count = 0;
  int size = 1;
  *neighbours = (NEIGHBOURS *) malloc(size * sizeof(NEIGHBOURS));
  if(*neighbours == NULL) {
    perror("malloc");
    exit(1);
  }
  for(int i = 4; i < argc; i++){
    sscanf(argv[i], "%60[^:]:%s", (*neighbours)[count].ip_addr, (*neighbours)[count].udp_port);
    sprintf((*neighbours)[count].addr_and_port, "%s:%s", (*neighbours)[count].ip_addr, (*neighbours)[count].udp_port);
    count++;
    if(size == count){
      size *= 2;
      NEIGHBOURS *temp = (NEIGHBOURS *) realloc(*neighbours, size * sizeof(NEIGHBOURS));
      if(temp == NULL) {
        fprintf(stderr, "Memory realloc()ation failed.\n");
        exit(1);
      }
      (*neighbours) = temp;
    }
  }
  return count;
}

void print_timetable(TIMETABLE *timetable, int NUM_TIMETABLES){
  for(int i = 0; i < NUM_TIMETABLES; i++){
    // departure-time,route-name,departing-from,arrival-time,arrival-station
    printf("%s, %s, %s, %s, %s\n", timetable[i].departure_time, timetable[i].route_name, timetable[i].departing_from, timetable[i].arrival_time, timetable[i].arrival_station);
  }
}

// Function to convert time string to minutes since midnight
int timeToMinutes(char *timeStr) {
    int hours, minutes;
    sscanf(timeStr, "%d:%d", &hours, &minutes);
    return hours * 60 + minutes;
}

int search_timetable(TIMETABLE *timetable, int NUM_TIMETABLES, char* starting_time, char *station_to_go){
  for(int i = 0; i < NUM_TIMETABLES - 1; i++) {
    if(timeToMinutes(starting_time) > timeToMinutes(timetable[i].departure_time)) continue;
    if(!strcmp(station_to_go, timetable[i].arrival_station))
      return i;
  }
  // if not found, search it again from "00:00" (PAST MIDNIGHT assuming the timetable is the same.)
  for(int i = 0; i < NUM_TIMETABLES - 1; i++) {
    // if(0 > timeToMinutes(timetable[i].departure_time)) continue; ALWAYS False, so skip.
    // Find the first with the desired arrival_station
    if(!strcmp(station_to_go, timetable[i].arrival_station))
      return i;
  }
  return -1;
}

void print_neighbours(NEIGHBOURS *neighbours, int NUM_NEIGHBOURS){
  for(int i = 0; i < NUM_NEIGHBOURS; i++) {
    printf("%s\n", neighbours[i].addr_and_port);
  }
}

void check_update_timetable(char* file, TIMETABLE** Timetable, STATION *Station, time_t latest){
  time_t f_time = get_file_mtime(file);

  double diff = difftime(f_time, latest);
  if (diff > 0) {
        printf("%s is newer than %s\n", f_time, latest);
    } else if (diff < 0) {
        printf("%s is older than %s\n", f_time, latest);
        printf("Updating the file\n");
        readtimetable(file, Station, Timetable);
    } else {
        printf("%s and %s have the same modification time\n", f_time, latest);
    }

    return 0;
}

time_t get_file_mtime(const char *filepath) {
    struct stat statbuf;

    // Get the file statistics
    if (stat(filepath, &statbuf) != 0) {
        perror("Failed to get file statistics");
        return -1;  // Return -1 to indicate error
    }

    // Return the modification time
    return statbuf.st_mtime;
}