#pragma once

#include "app.h"
#include "color.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
  struct {
    const char *text;
    const char *status;
    const char *icon;
    uint32_t size;
  } font;
  const char *action[APP_OPTIONS];
  struct {
    color_t overlay;
    color_t text;
    color_t status;
    color_t error;
    color_t selected;
    color_t button;
    color_t window;
  } color;
} config_t;

char *cfg_path(void);
void cfg_read(const char *path, config_t **cfg);
void cfg_debug(void);
