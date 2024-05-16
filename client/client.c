#include "client.h"
CLIENT_NODE *head_client = NULL;  // Head of the client list
QUERY queries[MAX_QUERIES];
int query_index = 0;  // This will be used to keep track of the next index to insert/overwrite
void add_client(int fd, QUERY query) {
    CLIENT_NODE *new_node = (CLIENT_NODE *)malloc(sizeof(CLIENT_NODE));
    if (new_node == NULL) {
        perror("Failed to allocate memory for new client");
        return;
    }
    new_node->fd = fd;
    new_node->query = query;
    new_node->next = head_client;
    head_client = new_node;
}

CLIENT_NODE *find_client(QUERY *q) {
    CLIENT_NODE *current = head_client;
    while (current != NULL) {
        if (!strcmp(current->query.destination, q->destination) && !strcmp(current->query.start_time, q->start_time)) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void remove_client(int fd) {
    CLIENT_NODE **ptr = &head_client;
    while (*ptr) {
        CLIENT_NODE *entry = *ptr;
        if (entry->fd == fd) {
            *ptr = entry->next;
            free(entry);
            return;
        }
        ptr = &entry->next;
    }
}

void add_query(QUERY new_query) {
    queries[query_index % MAX_QUERIES] = new_query;  // Overwrite if index exceeds MAX_QUERIES
    query_index = (query_index + 1) % MAX_QUERIES;  // Increment and wrap around if necessary
}

QUERY* search_query(const char* destination, const char* time) {
    QUERY *found_queries = malloc(sizeof(QUERY) * MAX_QUERIES);
    int count = 0;
    for (int i = 0; i < MAX_QUERIES; i++) {
        if (strcmp(queries[i].destination, destination) == 0 && strcmp(queries[i].start_time, time) == 0) {
            if (count < MAX_QUERIES) {
                found_queries[count++] = queries[i];
            }
        }
    }
    if (count == 0) {
        free(found_queries);
        return NULL;
    }
    return found_queries;  // Caller must free this memory
}

QUERY new_query(){
  QUERY res;
  res.answers[0] = '\0';
  res.destination[0] = '\0';
  res.start_time[0] = '\0';
  return res;
}