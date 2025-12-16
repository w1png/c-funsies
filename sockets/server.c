#include "shared.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

struct sockaddr_in server_addr;

#define MAX_SOCKETS 100

int all_sockets[MAX_SOCKETS];

void broadcast(const message *msg, int exclude_sockfd) {
  for (int i = 0; i < MAX_SOCKETS; i++) {
    if (all_sockets[i] == exclude_sockfd || all_sockets[i] == 0)
      continue;

    if (send(all_sockets[i], msg, MESSAGE_FIXED_SIZE, 0) !=
        MESSAGE_FIXED_SIZE) {
      perror("send failed");
    }
  }
}

void *handle_connection(void *arg) {
  int client_sockfd = (int)(long)arg;
  all_sockets[client_sockfd] = client_sockfd;

  message msg;
  ssize_t received;

  while ((received = recv(client_sockfd, &msg, MESSAGE_FIXED_SIZE,
                          MSG_WAITALL)) == MESSAGE_FIXED_SIZE) {
    uint16_t ulen = ntohs(msg.username_len);
    uint16_t mlen = ntohs(msg.message_len);

    if (ulen >= MAX_USERNAME)
      ulen = MAX_USERNAME - 1;
    if (mlen >= BUFFER_SIZE)
      mlen = BUFFER_SIZE - 1;

    msg.username[ulen] = '\0';
    msg.message[mlen] = '\0';

    print_message(&msg);
    broadcast(&msg, client_sockfd);
  }

  if (received == 0) {
    printf("disconnected\n");
  } else if (received == -1) {
    perror("recv failed");
  } else {
    printf("incomplete message received\n");
  }

  close(client_sockfd);
  all_sockets[client_sockfd] = 0;
  return NULL;
}

int main() {
  memset(all_sockets, 0, sizeof(all_sockets));
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
