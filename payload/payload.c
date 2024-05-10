#include "payload.h"

char* craft_payload(PAYLOAD p) {
    // This is the oversized malloc()ed payload, with this being able to support dynamically long payload of any size. 
    char* payload = (char*)malloc((p.hops * (MAX_NAME_LENGTH + MAX_PORT + INET6_ADDRSTRLEN + 1 + MAX_TIMESTRING) + 2 * MAX_NAME_LENGTH) * sizeof(char));
    sprintf(payload, "%d %d %d", p.found, p.hops, p.current);
    for (int i = 0; i < p.hops; i++) {
        strcat(payload, " ");
        strcat(payload, p.routes[i]);
    }

    for (int i = 0; i < p.hops; i++) {
      strcat(payload, " ");
      strcat(payload, p.address[i]);
    }
    strcat(payload, " ");
    strcat(payload, p.time);
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
        // Routes not found, allocate memory for routes based on hops + 1, current station to be added.
        for (int i = 0; i < (payload_struct->hops + 1); i++) {
            payload_struct->routes[i] = (char*)malloc(MAX_NAME_LENGTH * sizeof(char));
            payload_struct->address[i] = (char*)malloc((MAX_PORT + INET6_ADDRSTRLEN + 1) * sizeof(char));
        }
    } else {
        // Routes found, allocate memory for routes based on hops
        for (int i = 0; i < payload_struct->hops; i++) {
            payload_struct->routes[i] = (char*)malloc(MAX_NAME_LENGTH * sizeof(char));
            payload_struct->address[i] = (char*)malloc((MAX_PORT + INET6_ADDRSTRLEN + 1) * sizeof(char));
        }
    }

    token = strtok(payload_copy, " ");
    token = strtok(NULL, " ");
    token = strtok(NULL, " ");
    for (int i = 0; i < payload_struct->hops; i++) {
        token = strtok(NULL, " ");
        strcpy(payload_struct->routes[i], token);
    }

    for(int i = 0; i < payload_struct->hops; i++){
      token = strtok(NULL, " ");
      strcpy(payload_struct->address[i], token);
    }

    token = strtok(NULL, " ");
    strcpy(payload_struct->time, token);
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
    printf("Number of routes traversed: %d\n", payload_struct.hops);
    printf("Stations: ");
    for (int i = 0; i < payload_struct.hops; i++) {
        printf("%s ", payload_struct.routes[i]);
        printf("%s ", payload_struct.address[i]);
    }
    printf("\nTime: %s\n", payload_struct.time);
    printf("Destination: %s\n", payload_struct.destination);
    printf("Source: %s\n", payload_struct.source);
}
// int main() {
//     PAYLOAD payload_struct;
//     char payload_string[] = "0 3 3 StationA StationB StationC localhost:1002 localhost:1003 localhost:1004 10:45 StationX StationA;";
//     // Loading payload string into payload struct
//     load_payload(&payload_struct, payload_string);

//     print_payload(payload_struct);

//     // Free allocated memory for stations
//     for (int i = 0; i < payload_struct.hops; i++) {
//         free(payload_struct.routes[i]);
//         free(payload_struct.address[i]);
//     }
//     int found = 1;
//     int hops = 7;
//     char *routes[MAX_NAME_LENGTH] = {"StationA", "StationC", "StationD", "StationE", "StationZ", "StationG", "StationX"};
//     char *address[MAX_PORT + INET6_ADDRSTRLEN + 1] = {"localhost:1001", "localhost:1002", "localhost:1003", "localhost:1005", "localhost:1006", "localhost:1008", "localhost:1009"};
//     char time[] = "12:12";
//     char source[] = "StationA";
//     char destination[] = "StationX";

//     payload_struct.found = found;
//     payload_struct.hops = hops;
//     payload_struct.current = hops;
//     memcpy(payload_struct.routes, routes, MAX_NAME_LENGTH * sizeof(char) * hops);
//     memcpy(payload_struct.address, address, (MAX_PORT + INET6_ADDRSTRLEN + 1) * sizeof(char) * hops);
//     strcpy(payload_struct.time, time);
//     strcpy(payload_struct.source, source);
//     strcpy(payload_struct.destination, destination);
    
    

//     char *newpayload = craft_payload(payload_struct);
//     load_payload(&payload_struct, newpayload);
//     print_payload(payload_struct);
//     return 0;
// }
