#include "lib/shared.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int exit_code;

  char *output;
  size_t output_size;
} command_result;

#define COMMAND_MAX_OUTPUT_SIZE 256

#define KB_SIZE 1024
#define MB_SIZE (KB_SIZE * KB_SIZE)
#define GB_SIZE (MB_SIZE * KB_SIZE)
#define TB_SIZE (GB_SIZE * KB_SIZE)

enum os_type {
  LINUX,
  MACOS,
};

#define os_type_to_string(type)                                                \
  (type == LINUX ? "Linux" : (type == MACOS ? "Darwin" : "Unknown OS"))

char *format_size(uint64_t size) {
  char *result = malloc(sizeof(char) * 256);
  if (size < KB_SIZE) {
    sprintf(result, "%llu B", size);
  } else if (size < MB_SIZE) {
    sprintf(result, "%.2llu KB", size / KB_SIZE);
  } else if (size < GB_SIZE) {
    sprintf(result, "%.2llu MB", size / MB_SIZE);
  } else {
    sprintf(result, "%.2llu GB", size / GB_SIZE);
  }

  return result;
}

command_result *run_command(char *command) {
  command_result *result = malloc(sizeof(command_result));

  FILE *pipe = popen(command, "r");
  if (pipe == NULL) {
    result->exit_code = 1;
    return result;
  }

  size_t capacity = COMMAND_MAX_OUTPUT_SIZE;
  size_t length = 0;
  result->output = malloc(capacity);
  while (fgets(result->output + length, capacity - length, pipe)) {
    length += strlen(result->output + length);
    if (capacity - length < 128) {
      capacity *= 2;
      result->output = realloc(result->output, capacity);
    }
  }
  result->output[length] = '\0';
  result->output_size = length;

  pclose(pipe);

  return result;
}

typedef struct {
  enum os_type type;
  char *hostname;
  char *os_version;
} os_info;

os_info *get_uname(void) {
  command_result *result = run_command("uname -a");
  if (result->exit_code != 0) {
    err("Failed to get uname");
    return NULL;
  }

  char *os = malloc(sizeof(char) * 256);
  char *os_version = malloc(sizeof(char) * 256);
  char *hostname = malloc(sizeof(char) * 256);

  sscanf(result->output, "%s %s %s", os, hostname, os_version);

  os_info *os_i = malloc(sizeof(os_info));
  os_i->os_version = os_version;
  os_i->hostname = hostname;
  if (strcmp(os, "Linux") == 0) {
    os_i->type = LINUX;
  } else if (strcmp(os, "Darwin") == 0) {
    os_i->type = MACOS;
  } else {
    err("Unknown OS");
  }

  free(result->output);
  free(result);

  return os_i;
}

typedef struct {
  uint64_t total;
  uint64_t free;
  uint64_t used;
} memory_info;

typedef struct {
  char *username;
  os_info *os;
  memory_info mem;
} system_info;

memory_info *get_memory_info_linux(void) {
  memory_info *info = malloc(sizeof(memory_info));
  info->total = 0;
  info->free = 0;
  info->used = 0;
  command_result *result = run_command("free -m");
  if (result->exit_code != 0) {
    err("Failed to get memory info");
    return NULL;
  }

  char *line = strtok(result->output, "\n");
  while (line != NULL) {
    if (strstr(line, "Mem:") != NULL) {
      sscanf(line, "Mem: %llu %llu %llu", &info->total, &info->free,
             &info->used);
    }
    line = strtok(NULL, "\n");
  }
  free(result->output);
  free(result);

  return info;
}

memory_info *get_memory_info_darwin(void) {
  memory_info *info = malloc(sizeof(memory_info));
  info->total = 0;
  info->free = 0;
  info->used = 0;

  command_result *hw_memsize_result = run_command("sysctl hw.memsize");
  if (hw_memsize_result->exit_code != 0) {
    err("Failed to get memory info");
    return NULL;
  }

  sscanf(hw_memsize_result->output, "hw.memsize: %llu", &info->total);

  free(hw_memsize_result->output);
  free(hw_memsize_result);

  command_result *page_size_result = run_command("sysctl hw.pagesize");
  if (page_size_result->exit_code != 0) {
    err("Failed to get memory info");
    return NULL;
  }
  uint64_t page_size;
  sscanf(page_size_result->output, "hw.pagesize: %llu", &page_size);
  free(page_size_result->output);
  free(page_size_result);

  command_result *vm_stat_result = run_command("vm_stat");
  if (vm_stat_result->exit_code != 0) {
    err("Failed to get memory info");
    return NULL;
  }

  uint64_t pages_free = 0;
  uint64_t pages_speculative = 0;
  uint64_t pages_wired = 0;
  char *line = strtok(vm_stat_result->output, "\n");
  while (line != NULL) {
    if (strstr(line, "Pages free:") != NULL) {
      sscanf(line, "Pages free: %llu", &pages_free);
    } else if (strstr(line, "Pages speculative:") != NULL) {
      sscanf(line, "Pages speculative: %llu", &pages_speculative);
    } else if (strstr(line, "Pages wired down:") != NULL) {
      sscanf(line, "Pages wired down: %llu", &pages_wired);
    }
    line = strtok(NULL, "\n");
  }
  free(vm_stat_result->output);
  free(vm_stat_result);

  info->total = info->total;
  info->free = (pages_free + pages_speculative) * page_size;
  info->used = info->total - info->free;

  return info;
}

memory_info *get_memory_info(enum os_type ost) {
  if (ost == LINUX) {
    return get_memory_info_linux();
  } else {
    return get_memory_info_darwin();
  }

  return NULL;
}

char *get_username(void) {
  command_result *result = run_command("whoami");
  if (result->exit_code != 0) {
    err("Failed to get username");
    return NULL;
  }

  char *username = strdup(strtok(result->output, "\n"));
  free(result->output);
  free(result);

  return username;
}

system_info *get_system_info(void) {
  system_info *info = malloc(sizeof(system_info));

  info->username = get_username();
  info->os = get_uname();

  memory_info *mem = get_memory_info(info->os->type);
  info->mem = *mem;

  return info;
}

#define COLOR_RESET "\x1b[0m"
#define COLOR_PRIMARY "\x1b[38;5;165m"
#define COLOR_LABEL COLOR_PRIMARY

void print_system_info(system_info *info) {
  printf(COLOR_PRIMARY);
  printf("  ,d88b.d88b,\t");

  // username@hostname
  printf("%s", info->username);
  printf(COLOR_RESET);
  printf("@");
  printf(COLOR_PRIMARY);
  printf("%s\n", info->os->hostname);

  printf("  88888888888\t");

  // OS
  printf(COLOR_LABEL);
  printf("OS: ");
  printf(COLOR_RESET);
  printf("%s (version %s)\n", os_type_to_string(info->os->type),
         info->os->os_version);
  printf(COLOR_PRIMARY);

  printf("  `Y8888888Y'\t");

  // memory
  printf(COLOR_LABEL);
  printf("Memory: ");
  printf(COLOR_RESET);
  printf("%s/%s\n", format_size(info->mem.used), format_size(info->mem.total));
  printf(COLOR_PRIMARY);

  printf("    `Y888Y'\n");
  printf("      `Y'\n");
  printf(COLOR_RESET);
}

int main(void) {
  system_info *info = get_system_info();
  print_system_info(info);
  return 0;
}
