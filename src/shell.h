#pragma once
#include "ctx.h"
#include "result.h"
#include <stdint.h>
#include <wayland-client-protocol.h>

typedef enum {
  SHL_KEY_UNKNOWN,
  SHL_KEY_UP,
  SHL_KEY_DOWN,
  SHL_KEY_LEFT,
  SHL_KEY_RIGHT,
  SHL_KEY_CONFIRM,
  SHL_KEY_CANCEL,
} shl_key_t;

typedef enum {
  SHL_KBD_EVENT_KEYUP,
  SHL_KBD_EVENT_KEYDOWN,
} shl_kbd_event_t;

typedef enum {
  SHL_PTR_BTN_UNKNOWN,
  SHL_PTR_BTN_LEFT,
} shl_ptr_btn_t;

typedef enum {
  SHL_PTR_EVENT_MOVE,
  SHL_PTR_EVENT_CLICK,
} shl_ptr_event_t;

typedef struct {
  void (*draw)(uint32_t w, uint32_t h, ctx_t **c);
  bool (*key)(shl_kbd_event_t evt, shl_key_t key);
  // x and y are surface-local; wl_pointer.button carries no coordinates of its
  // own, so a click reports the last position seen on enter or motion
  bool (*pointer)(shl_ptr_event_t evt, shl_ptr_btn_t btn, double x, double y);
} shl_callbacks_t;

typedef struct {
  struct wl_shm *shm;
  shl_callbacks_t callbacks;
} shl_shell_t;

result_t shl_create(shl_callbacks_t cbs, shl_shell_t **shl);

void shl_draw(void);
result_t shl_run(void);
