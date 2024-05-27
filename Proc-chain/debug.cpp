#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <iostream>

enum { MAX_ARGS_COUNT = 256, MAX_CHAIN_LINKS_COUNT = 256 };

typedef struct {
  char* command;
  uint64_t argc = 0;
  char* argv[MAX_ARGS_COUNT];
} chain_link_t;

typedef struct {
  uint64_t chain_links_count;
  chain_link_t chain_links[MAX_CHAIN_LINKS_COUNT];
} chain_t;

void create_chain(char* command, chain_t* chain) {
  chain->chain_links_count = 0;
  char* one_token = strtok(command, " ");
  while (one_token != NULL && chain->chain_links_count < MAX_CHAIN_LINKS_COUNT) {
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

int main() {
  chain_t chain;
  char system[] = "ls -l";
  create_chain(system, &chain);
  for (uint64_t ind = 0; ind < chain.chain_links_count; ++ind) {
    std::cout << chain.chain_links[ind].command << '\n';
   if (chain.chain_links[ind].argv[0] != NULL) {
      std::cout << chain.chain_links[ind].argv[0] << '\n';
    }
    if (chain.chain_links[ind].argv[1] != NULL) {
      std::cout << chain.chain_links[ind].argv[1] << '\n';
    }
  }
}