#include "shared.h"
#include <stdio.h>

void print_message(const message *message_buf) {
  printf("\33[2K\r");
  printf("\n=== New message from %s ===\n%s\n==================\n",
         message_buf->username, message_buf->message);
}
