#pragma once

#include "result.h"
#include "ui.h"
#include <stdint.h>

typedef struct {
    struct {
      char* sans;
      char* mono;
      char* icon;
      uint32_t size;
    } font;
    struct {
      const char* lock;
      const char* suspend;
      const char* hibernate;
      const char* logout;
      const char* reboot;
      const char* shutdown;
    } commands;
    struct {
      color_t overlay;
      color_t text;
      color_t error;
    } color;
} config_t;

result_t config_read(config_t **config);

