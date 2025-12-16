#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
  char *initialPath = getenv("PATH");
  if (initialPath == NULL) {
    printf("PATH is not set\n");
    return 1;
  }

  for (;;) {
    printf("$ ");
    fflush(stdout);
    char *input = malloc(1024);
    fgets(input, 1024, stdin);
    if (strcmp(input, "exit\n") == 0) {
      free(input);
      return 0;
    }

    if (input[0] == '\n') {
      free(input);
      continue;
    }

    input[strlen(input) - 1] = '\0';

    char **argv = malloc(32 * sizeof(char *));
    int argc = 0;
    char *curr = strtok(input, " ");
    while (curr && argc < 31) {
      argv[argc++] = curr;
      curr = strtok(NULL, " ");
    }
    argv[argc] = NULL;

    if (argc == 0) {
      free(input);
      continue;
    }

    char *command = argv[0];
    int args_count = argc - 1;

    char *program_path = {0};

    char *path = strdup(initialPath);
    char *curr_path = strtok(path, ":");
    while (curr_path) {
      char *full_path = malloc(strlen(curr_path) + strlen(command) + 2);
      sprintf(full_path, "%s/%s", curr_path, command);

      FILE *file = fopen(full_path, "r");
      if (file) {
        fclose(file);
        program_path = full_path;
        break;
      }

      curr_path = strtok(NULL, ":");
    }

    if (program_path == NULL) {
      printf("Command not found: %s\n", command);
      continue;
    }

    pid_t pid = fork();
    if (pid == 0) {
      execvp(program_path, argv);
    } else {
      waitpid(pid, NULL, 0);
    }

    free(input);
  }

  return 0;
}
