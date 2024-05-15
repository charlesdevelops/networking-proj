#include "../macros/macros.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAX_QUERIES 100
typedef struct {
    char start_time[MAX_TIMESTRING];
    char destination[MAX_NAME_LENGTH];
    char answers[MAXDATASIZE];
} QUERY;

typedef struct client_node {
    int fd;              // File descriptor for the TCP connection
    QUERY query;         // Associated query
    struct client_node *next;
} CLIENT_NODE;

extern CLIENT_NODE *head_client; // Head of the client list

extern QUERY queries[MAX_QUERIES];
extern int query_index;// This will be used to keep track of the next index to insert/overwrite

void add_client(int fd, QUERY query);
CLIENT_NODE *find_client(QUERY *q);
void remove_client(int fd);
void add_query(QUERY new_query);
QUERY* search_query(const char* destination, const char* time);
QUERY new_query(void);