 #include "serverTCP.h"
 #include "response_http.h"
 #define BACKLOG 10
 struct addrinfo *getAddrInfo(char *port){
  int rv;
  struct addrinfo hints, *servers;
  // construct hints
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_socktype = SOCK_STREAM;
  if ((rv = getaddrinfo(NULL, port, &hints, &servers)) != 0){
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return NULL;
  }
  return servers;
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
  if ( sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr); // convert to IPv4 sockaddr_in
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void service_TCP(int sockfd){
  int new_fd;
  struct sockaddr_storage client_addr;
  socklen_t sin_size;
  char query[MAXDATASIZE], *response;
  char s[INET6_ADDRSTRLEN];

  sin_size = sizeof client_addr;
    new_fd = accept(sockfd, (struct sockaddr *) &client_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      return;
    }

    inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *) &client_addr), s, sizeof s);
    printf("server: got connection from %s\n", s);

    if (recv(new_fd, query, MAXDATASIZE, 0) <= 0){
        perror("recv");
        return;
    } printf("Received:\n\n%s\n", query);

    response = calloc((strlen(HTTP_200_RESPONSE) + strlen(HTML_200_MESSAGE) + 5), sizeof(char));

    if (response == NULL) perror("calloc");
    sprintf(response, "%s\r\n%s", HTTP_200_RESPONSE,HTML_200_MESSAGE);
    if (send(new_fd, response, strlen(response), 0) == -1) {
        perror("send");
        exit(EXIT_FAILURE);
    };

    printf("connection %d in the main while has been closed\n", new_fd);
    close(new_fd);
};


void setup_TCP(int *sockfd, char *TCP_PORT){
  struct addrinfo *servinfo, *p;

  // fill the addrInfo and return linked list of addrinfo(s).
  servinfo = getAddrInfo(TCP_PORT);

  p = servinfo;
  for(p = servinfo; p != NULL; p=servinfo->ai_next){
    if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      exit(EXIT_FAILURE);
    }

    int yes=1;
    //char yes='1'; // Solaris people use this

    // lose the pesky "Address already in use" error message
    if (setsockopt(*sockfd ,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof yes) == -1) {
      perror("setsockopt");
      exit(1);
    } 
    if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1){
      close(*sockfd);
      perror("server: bind");
      exit(EXIT_FAILURE);
    }

    break;
  }

  freeaddrinfo(servinfo); // no need with the rest

  if (listen(*sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
}
