#ifndef SHARED_H
#define SHARED_H

#include <stddef.h>
#include <stdint.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_USERNAME 256

#pragma pack(push, 1)
typedef struct {
  uint16_t username_len;
  uint16_t message_len;
  char username[MAX_USERNAME];
  char message[BUFFER_SIZE];
} message;
#pragma pack(pop)

#define MESSAGE_FIXED_SIZE (sizeof(message))

void print_message(const message *msg);

#endif
