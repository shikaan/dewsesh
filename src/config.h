#pragma once

#include "color.h"
#include <stdint.h>

typedef struct {
  struct {
    const char *text;
    const char *status;
    const char *icon;
    uint32_t size;
  } font;
  struct {
    const char *lock;
    const char *suspend;
    const char *hibernate;
    const char *logout;
    const char *reboot;
    const char *shutdown;
  } actions;
  struct {
    color_t overlay;
    color_t text;
    color_t error;
    color_t selected;
    color_t button;
    color_t status;
  } color;
} config_t;

char *cfg_path(void);
void cfg_read(const char *path, config_t **cfg);
