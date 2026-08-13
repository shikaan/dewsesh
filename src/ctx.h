#pragma once
#include "cairo.h"
#include "types.h"
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

result_t ctx_create(ctx_t *c, size_t width, size_t height,
                    struct wl_shm *wl_shm);
void ctx_destroy(ctx_t **c);
