#include "payload.h"

char* craft_payload(PAYLOAD p) {
    // This is the oversized malloc()ed payload, with this being able to support dynamically long payload of any size. 
    char* payload = (char*)malloc((p.hops * (MAX_NAME_LENGTH + MAX_PORT + INET6_ADDRSTRLEN + 1 + MAX_TIMESTRING + MAX_NAME_LENGTH) + 2 * MAX_NAME_LENGTH) * sizeof(char));
    sprintf(payload, "%d %d %d", p.found, p.hops, p.current);
    for (int i = 0; i < p.hops; i++) {
        strcat(payload, " ");
        strcat(payload, p.stations[i]);
    }
    
    for (int i = 0; i < p.hops; i++) {
      strcat(payload, " ");
      strcat(payload, p.address[i]);
    }

    for (int i = 0; i < p.hops; i++){
      strcat(payload, " ");
      strcat(payload, p.routes[i]);
    }

    for (int i = 0; i < p.hops; i++){
      strcat(payload, " ");
      strcat(payload, p.time[i]);
    }
    strcat(payload, " ");
    strcat(payload, p.destination);
    strcat(payload, " ");
    strcat(payload, p.source);
    strcat(payload, ";"); // end
    return payload;
}

void load_payload(PAYLOAD *payload_struct, char* payload_string) {
    char* token;
    char* payload_copy = strdup(payload_string); // Create a copy to avoid modifying original string
    sscanf(payload_copy, "%d %d %d", &payload_struct->found, &payload_struct->hops, &payload_struct->current);

    if (payload_struct->found == 0) {
    // stations not found, allocate memory for stations based on hops + 1, current station to be added.
    int hops = payload_struct->hops + 1;
    payload_struct->stations = (char**) malloc(sizeof(char*) * hops);
    payload_struct->address = (char**) malloc(sizeof(char*) * hops);
    payload_struct->routes = (char**) malloc(sizeof(char*) * hops);
    payload_struct->time = (char**) malloc(sizeof(char*) * hops);
    for (int i = 0; i < hops; i++) {
        payload_struct->stations[i] = (char*)malloc(MAX_NAME_LENGTH * sizeof(char));
        payload_struct->address[i] = (char*)malloc((MAX_PORT + INET6_ADDRSTRLEN + 1) * sizeof(char));
        payload_struct->routes[i] = (char*)malloc(MAX_NAME_LENGTH * sizeof(char));
        payload_struct->time[i] = (char*)malloc(MAX_TIMESTRING * sizeof(char));
    }
} else {
    // stations found, allocate memory for stations based on hops
    int hops = payload_struct->hops;
    payload_struct->stations = (char**) malloc(sizeof(char*) * hops);
    payload_struct->address = (char**) malloc(sizeof(char*) * hops);
    payload_struct->routes = (char**) malloc(sizeof(char*) * hops);
    payload_struct->time = (char**) malloc(sizeof(char*) * hops);
    
    for (int i = 0; i < hops; i++) {
        payload_struct->stations[i] = (char*)malloc(MAX_NAME_LENGTH * sizeof(char));
        payload_struct->address[i] = (char*)malloc((MAX_PORT + INET6_ADDRSTRLEN + 1) * sizeof(char));
        payload_struct->routes[i] = (char*)malloc(MAX_NAME_LENGTH * sizeof(char));
        payload_struct->time[i] = (char*)malloc(MAX_TIMESTRING * sizeof(char));
    }
}


    token = strtok(payload_copy, " ");
    token = strtok(NULL, " ");
    token = strtok(NULL, " ");
    for (int i = 0; i < payload_struct->hops; i++) {
        token = strtok(NULL, " ");
        strcpy(payload_struct->stations[i], token);
    }

    for (int i = 0; i < payload_struct->hops; i++){
      token = strtok(NULL, " ");
      strcpy(payload_struct->address[i], token);
    }

    for (int i = 0; i < payload_struct->hops; i++){
      token = strtok(NULL, " ");
      strcpy(payload_struct->routes[i], token);
    }

    for (int i = 0; i < payload_struct->hops; i++){
      token = strtok(NULL, " ");
      strcpy(payload_struct->time[i], token);
    }
    token = strtok(NULL, " ");
    strcpy(payload_struct->destination, token);
    token = strtok(NULL, ";");
    strcpy(payload_struct->source, token);

    free(payload_copy); // Free the copied string
}

void print_payload(PAYLOAD payload_struct){

    // Accessing payload struct attributes
    printf("Found: %d\n", payload_struct.found);
    printf("Current: %d\n", payload_struct.current);
    printf("Number of stations traversed: %d\n", payload_struct.hops);
    for (int i = 0; i < payload_struct.hops; i++) {
        printf("%s ", payload_struct.stations[i]);
        printf("%s ", payload_struct.address[i]);
        printf("%s ", payload_struct.routes[i]);
        printf("%s \n", payload_struct.time[i]);
    }
    
    printf("Destination: %s\n", payload_struct.destination);
    printf("Source: %s\n\n", payload_struct.source);
}

// int main() {
//     PAYLOAD payload_struct;
//     char payload_string[] = "1 5 5 StationA StationB StationC StationD StationE localhost:1002 localhost:1003 localhost:1004 localhost:1005 localhost:1006 BASE A_B B_C C_D D_E 10:45 11:45 12:45 13:45 14:45 StationX StationA;";
//     // Loading payload string into payload struct
//     load_payload(&payload_struct, payload_string);

//     print_payload(payload_struct);
    
//     PAYLOAD new_struct;
//     char another_payload[] = "0 7 7 StationA StationB StationC StationD StationE StationF StationX localhost:1001 localhost:1002 localhost:1003 localhost:1004 localhost:1005 localhost:1006 localhost:1007 BASE A_B B_C C_D D_E E_F F_X 10:45 11:45 12:45 10:45 11:45 12:45 11:99 StationX StationA;";

//     load_payload(&new_struct, another_payload);
//     print_payload(new_struct);
//     return 0;
// }
