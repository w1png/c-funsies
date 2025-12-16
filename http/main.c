#include "http.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define PORT 8080

int TTL_SECONDS = 60;
int last_request_timestamps[1024] = {0};

long file_contents_len;

time_t get_time() {
  time_t t;
  time(&t);
  return t;
}

void send_http_response(int client_sockfd, http_response response) {
  char *response_str = http_response_to_string(response);
  ssize_t bytes_sent =
      send(client_sockfd, response_str, strlen(response_str), 0);
  if (bytes_sent < 0) {
    perror("send failed");
  } else if (bytes_sent == 0) {
    printf("Client closed connection after receiving response\n");
  } else if ((size_t)bytes_sent < strlen(response_str)) {
    printf("Warning: partial send, only %zd of %zu bytes sent\n", bytes_sent,
           strlen(response_str));
  }

  free(response_str);
}

struct sockaddr_in server_addr;

char *get_file_contents(char *path) {
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file);

  char *file_contents = malloc(file_size + 1);
  fread(file_contents, file_size, 1, file);
  file_contents[file_size] = '\0';
  fclose(file);

  return file_contents;
}

void handle_request(http_request request, int client_sockfd) {
  http_response response;
  char *file_contents = NULL;
  char *content_length_header = NULL;
  char **headers = NULL;
  printf("handling request. path: %s\n", request.path);

  if (strcmp(request.path, "/") == 0) {
    response = redirect_to_response("/index.html");
  } else {
    file_contents = get_file_contents(request.path + 1);
    if (file_contents == NULL) {
      response = not_found_response();
    } else {
      content_length_header = malloc(BUFFER_SIZE);
      sprintf(content_length_header, "Content-Length: %ld",
              strlen(file_contents));
      headers = malloc(2 * sizeof(char *));
      headers[0] = "Content-Type: text/html";
      headers[1] = content_length_header;

      response = build_http_response(200, headers, 2, file_contents);
    }
  }

  send_http_response(client_sockfd, response);

  free(content_length_header);
  free(headers);
}

void *handle_connection(void *arg) {
  int client_sockfd = (int)(long)arg;

  char buffer[BUFFER_SIZE];
  ssize_t received = recv(client_sockfd, &buffer, BUFFER_SIZE, 0);

  if (received == 0) {
    printf("disconnected\n");
  } else if (received == -1) {
    perror("recv failed");
  }

  http_request request = parse_http_request(buffer);
  handle_request(request, client_sockfd);

  return NULL;
}

int main() {
  printf("Starting server...\n");

  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(sockfd, 10) == -1) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Created socket %d\n", sockfd);

  while (1) {
    printf("Ready to accept new connections...\n");

    int client_sockfd;
    if ((client_sockfd = accept(sockfd, NULL, NULL)) == -1) {
      perror("accept failed");
      exit(EXIT_FAILURE);
    }

    pthread_t thread;

    if (pthread_create(&thread, NULL, handle_connection,
                       (void *)(long)client_sockfd) != 0) {
      perror("pthread_create failed");
      exit(EXIT_FAILURE);
    }
  }

  close(sockfd);

  return 0;
}
