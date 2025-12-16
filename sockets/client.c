#include "shared.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

char username[BUFFER_SIZE];

void *handle_send(void *arg) {
  int sockfd = (int)(long)arg;
  message msg;

  while (1) {
    printf("Enter message: ");
    fflush(stdout);

    msg.username_len = strlen(username);
    msg.message_len = 0;
    msg.username[0] = '\0';
    msg.message[0] = '\0';

    if (fgets(msg.message, BUFFER_SIZE, stdin) == NULL) {
      break;
    }

    msg.message[strcspn(msg.message, "\n")] = '\0';
    msg.message_len = strlen(msg.message);

    if (msg.message_len == 0) {
      continue;
    }

    strncpy(msg.username, username, MAX_USERNAME - 1);
    msg.username[MAX_USERNAME - 1] = '\0';
    msg.username_len = strlen(msg.username);

    msg.username_len = htons(msg.username_len);
    msg.message_len = htons(msg.message_len);

    if (send(sockfd, &msg, MESSAGE_FIXED_SIZE, 0) != MESSAGE_FIXED_SIZE) {
      perror("send failed");
      break;
    }
  }
  return NULL;
}

void *handle_receive(void *arg) {
  int sockfd = (int)(long)arg;
  message message_buf;
  ssize_t len;

  while ((len = recv(sockfd, &message_buf, MESSAGE_FIXED_SIZE, MSG_WAITALL)) ==
         MESSAGE_FIXED_SIZE) {
    uint16_t ulen = ntohs(message_buf.username_len);
    uint16_t mlen = ntohs(message_buf.message_len);

    if (ulen >= MAX_USERNAME)
      ulen = MAX_USERNAME - 1;
    if (mlen >= BUFFER_SIZE)
      mlen = BUFFER_SIZE - 1;

    message_buf.username[ulen] = '\0';
    message_buf.message[mlen] = '\0';

    print_message(&message_buf);

    printf("Enter message: ");
    fflush(stdout);
  }

  if (len == 0) {
    printf("\nServer disconnected.\n");
  } else {
    perror("recv");
  }
  return NULL;
}

int main() {
  printf("Starting client...\n");
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  printf("Enter your username: ");
  scanf("%s", username);
  printf("\n");
  printf("\33[2K\r");

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) ==
      -1) {
    perror("connect failed");
    exit(EXIT_FAILURE);
  }

  pthread_t send_thread;
  pthread_t receive_thread;

  if (pthread_create(&send_thread, NULL, handle_send, (void *)(long)sockfd) !=
      0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  if (pthread_create(&receive_thread, NULL, handle_receive,
                     (void *)(long)sockfd) != 0) {
    perror("pthread_create failed");
    exit(EXIT_FAILURE);
  }

  pthread_join(send_thread, NULL);
  pthread_join(receive_thread, NULL);

  close(sockfd);

  return 0;
}
