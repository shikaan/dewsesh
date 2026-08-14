#pragma once

#include "cairo.h"
#include "result.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <wayland-client.h>

typedef struct {
  bool busy;
  size_t width, height;

  struct {
    struct wl_buffer *buffer;
  } wl;

  struct {
    void *buf;
    size_t len;
  } shm;

  struct {
    cairo_surface_t *target;
    cairo_t *ctx;
  } cairo;
} ctx_t;

result_t ctx_init(uint32_t w, uint32_t h, struct wl_shm *shm);

result_t ctx_get(uint32_t w, uint32_t h, ctx_t **ctx);
