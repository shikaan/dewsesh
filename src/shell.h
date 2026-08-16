#pragma once
#include "ctx.h"
#include "result.h"
#include <stdint.h>
#include <wayland-client-protocol.h>

typedef enum {
  SHL_KEY_UNKNOWN,
  SHL_KEY_UP,
  SHL_KEY_DOWN,
  SHL_KEY_SELECT,
  SHL_KEY_EXIT,
} shl_key_t;

typedef enum {
  SHL_KBD_EVENT_KEYUP,
  SHL_KBD_EVENT_KEYDOWN,
} shl_kbd_event_t;

typedef struct {
  void (*draw)(uint32_t w, uint32_t h, ctx_t **c);
  bool (*key)(shl_kbd_event_t evt, shl_key_t key);
} shl_callbacks_t;

typedef struct {
  struct wl_shm *shm;
  shl_callbacks_t callbacks;
} shl_shell_t;

result_t shl_create(shl_callbacks_t cbs, shl_shell_t **shl);

void shl_draw(void);
void shl_run(void);
