#pragma once
#include "ctx.h"
#include "result.h"
#include <stdint.h>
#include <wayland-client-protocol.h>

typedef struct {
  void (*draw)(uint32_t w, uint32_t h, ctx_t **c);
} callbacks_t;

typedef struct {
  struct wl_shm *shm;
  callbacks_t callbacks;
} shell_t;

result_t shl_init(callbacks_t cbs, shell_t **shl);

void shl_draw(void);
void shl_run(void);
