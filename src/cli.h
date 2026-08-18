#pragma once

#include <stdbool.h>

typedef struct {
  bool debug;
  const char *config;
} cli_opts_t;

void cli_parse(int argc, char *const *argv, cli_opts_t **opts);
