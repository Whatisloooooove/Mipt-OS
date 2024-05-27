#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

enum { MAX_ARGS_COUNT = 256, MAX_CHAIN_LINKS_COUNT = 256 };

typedef struct {
  char* command;
  uint64_t argc;
  char* argv[MAX_ARGS_COUNT];
} chain_link_t;

typedef struct {
  uint64_t chain_links_count;
  chain_link_t chain_links[MAX_CHAIN_LINKS_COUNT];
} chain_t;

void create_chain(char* command, chain_t* chain) {
  chain->chain_links_count = 0;
  char* one_token = strtok(command, " ");
  while (one_token != NULL &&
         chain->chain_links_count < MAX_CHAIN_LINKS_COUNT) {
    ++chain->chain_links_count;
    chain_link_t* current_link =
        &chain->chain_links[chain->chain_links_count - 1];
    while (one_token != NULL && current_link->argc < MAX_ARGS_COUNT) {
      if (strcmp(one_token, "|") == 0) {
        break;
      }
      if (current_link->argc == 0) {
        current_link->command = strdup(one_token);
      }
      current_link->argv[current_link->argc] = strdup(one_token);
      ++current_link->argc;
      one_token = strtok(NULL, " ");
    }
    --current_link->argc;
    if (one_token != NULL) {
      one_token = strtok(NULL, " ");
    }
  }
}

void run_chain(chain_t* chain) {
  int prev_pipe[2];
  int curr_pipe[2];

  for (uint64_t ind = 0; ind < chain->chain_links_count; ++ind) {
    if (pipe(curr_pipe) == -1) {
      exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
      close(curr_pipe[0]);
      close(curr_pipe[1]);
      exit(EXIT_FAILURE);
    } else if (pid == 0) {
      if (ind > 0) {
        dup2(prev_pipe[0], STDIN_FILENO);
        close(prev_pipe[0]);
        close(prev_pipe[1]);
      }
      if (ind < chain->chain_links_count - 1) {
        dup2(curr_pipe[1], STDOUT_FILENO);
        close(curr_pipe[0]);
        close(curr_pipe[1]);
      }

      execvp(chain->chain_links[ind].command, chain->chain_links[ind].argv);
      exit(EXIT_FAILURE);
    } else {
      if (ind > 0) {
        close(prev_pipe[0]);
        close(prev_pipe[1]);
      }
      prev_pipe[0] = curr_pipe[0];
      prev_pipe[1] = curr_pipe[1];
    }
  }

  close(prev_pipe[0]);
  close(prev_pipe[1]);

  for (uint64_t ind = 0; ind < chain->chain_links_count; ++ind) {
    wait(NULL);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 2) {
    return -1;
  }
  chain_t chain;
  create_chain(argv[1], &chain);
  run_chain(&chain);
}
