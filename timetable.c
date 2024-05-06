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