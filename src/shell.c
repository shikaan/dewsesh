#include "shell.h"
#include "ctx.h"
#include "log.h"
#include "result.h"
#include "timer.h"
#include <assert.h>
#include <cursor-shape-v1.h>
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wlr-layer-shell-unstable-v1.h>

// stub for a symbol in the cursor-shape protocol we do not use
const struct wl_interface zwp_tablet_tool_v2_interface;

static shl_shell_t shell = {0};

static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface,
                            uint32_t version);
static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name);
static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *surf,
                                    uint32_t serial, uint32_t w, uint32_t h);
static void layer_surface_closed(void *data,
                                 struct zwlr_layer_surface_v1 *surf);

static void seat_capabilities(void *data, struct wl_seat *wl_seat,
                              uint32_t capabilities);
static void seat_name(void *data, struct wl_seat *wl_seat, const char *name);

static void keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                            uint32_t format, int32_t fd, uint32_t size);
static void keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, struct wl_surface *surface,
                           struct wl_array *keys);
static void keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, struct wl_surface *surface);
static void keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                         uint32_t serial, uint32_t time, uint32_t key,
                         uint32_t _key_state);
static void keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                               uint32_t serial, uint32_t mods_depressed,
                               uint32_t mods_latched, uint32_t mods_locked,
                               uint32_t group);
static void keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                                 int32_t rate, int32_t delay);

static void pointer_enter(void *data, struct wl_pointer *wl_pointer,
                          uint32_t serial, struct wl_surface *s,
                          wl_fixed_t surface_x, wl_fixed_t surface_y);
static void pointer_leave(void *data, struct wl_pointer *wl_pointer,
                          uint32_t serial, struct wl_surface *surface);

static void pointer_motion(void *data, struct wl_pointer *wl_pointer,
                           uint32_t time, wl_fixed_t surface_x,
                           wl_fixed_t surface_y);

static void pointer_button(void *data, struct wl_pointer *wl_pointer,
                           uint32_t serial, uint32_t time, uint32_t button,
                           uint32_t state);

static void pointer_axis(void *data, struct wl_pointer *wl_pointer,
                         uint32_t time, uint32_t axis, wl_fixed_t value);

static void pointer_frame(void *data, struct wl_pointer *wl_pointer);

static void pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                                uint32_t axis_source);

static void pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                              uint32_t time, uint32_t axis);

static void pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
                                  uint32_t axis, int32_t discrete);

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct zwlr_layer_shell_v1 *layer_shell = NULL;
static struct wl_seat *seat = NULL;
static struct wp_cursor_shape_manager_v1 *cursor_shape = NULL;

static struct wl_keyboard *keyboard = NULL;
static struct wl_pointer *pointer = NULL;
static struct wp_cursor_shape_device_v1 *cursor_shape_device = NULL;

static double pointer_x = 0;
static double pointer_y = 0;
static uint32_t pointer_serial = 0;
static shl_cursor_t cursor = SHL_CURSOR_DEFAULT;

static struct wl_surface *surface;
static struct zwlr_layer_surface_v1 *layer_surface;
static uint32_t surface_width = 0;
static uint32_t surface_height = 0;

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

static const struct wl_seat_listener seat_listener = {
    .capabilities = seat_capabilities,
    .name = seat_name,
};

static const struct wl_keyboard_listener keyboard_listener = {
    .keymap = keyboard_keymap,
    .enter = keyboard_enter,
    .leave = keyboard_leave,
    .key = keyboard_key,
    .modifiers = keyboard_modifiers,
    .repeat_info = keyboard_repeat_info,
};

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_enter,
    .leave = pointer_leave,
    .motion = pointer_motion,
    .button = pointer_button,
    .axis = pointer_axis,
    .frame = pointer_frame,
    .axis_source = pointer_axis_source,
    .axis_stop = pointer_axis_stop,
    .axis_discrete = pointer_axis_discrete,
};

static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *surf,
                                    uint32_t serial, uint32_t w, uint32_t h) {
  (void)data;
  log_debug("configure event: w=%u h=%u", w, h);
  zwlr_layer_surface_v1_ack_configure(surf, serial);

  if (w == 0 || h == 0) {
    // compositor hasn't settled on a size yet; wait for the next configure
    return;
  }

  if (w != surface_width || h != surface_height) {
    result_t res = ctx_init(w, h, shm);
    if (res != OK) {
      log_error("failed to (re)initialize rendering context", NULL);
      return;
    }
    surface_width = w;
    surface_height = h;
  }

  shl_draw();
}

static void layer_surface_closed(void *data,
                                 struct zwlr_layer_surface_v1 *surf) {
  (void)data;
  (void)surf;
  log_info("layer surface closed by compositor, exiting", NULL);
  exit(0);
}

static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface,
                            uint32_t version) {
  (void)data;
  (void)version; // FIXME: negotiate versions

  if (strcmp(interface, wl_compositor_interface.name) == 0) {
    compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
  } else if (strcmp(interface, wl_shm_interface.name) == 0) {
    shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
  } else if (strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
    layer_shell =
        wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface, 1);
  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    seat = wl_registry_bind(registry, name, &wl_seat_interface, 4);
  } else if (strcmp(interface, wp_cursor_shape_manager_v1_interface.name) == 0) {
    cursor_shape = wl_registry_bind(registry, name,
                                    &wp_cursor_shape_manager_v1_interface, 1);
  }
}

static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name) {
  (void)data;
  (void)registry;
  (void)name;
}

static void seat_capabilities(void *data, struct wl_seat *wl_seat,
                              uint32_t capabilities) {
  (void)data;
  if ((capabilities & WL_SEAT_CAPABILITY_KEYBOARD) && !keyboard) {
    keyboard = wl_seat_get_keyboard(wl_seat);
    wl_keyboard_add_listener(keyboard, &keyboard_listener, NULL);
  }

  if ((capabilities & WL_SEAT_CAPABILITY_POINTER) && !pointer) {
    pointer = wl_seat_get_pointer(wl_seat);
    wl_pointer_add_listener(pointer, &pointer_listener, NULL);

    if (cursor_shape) {
      cursor_shape_device =
          wp_cursor_shape_manager_v1_get_pointer(cursor_shape, pointer);
    }
  }
}

static void seat_name(void *data, struct wl_seat *wl_seat, const char *name) {
  (void)data;
  (void)wl_seat;
  (void)name;
}

static void keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                            uint32_t format, int32_t fd, uint32_t size) {
  (void)data;
  (void)wl_keyboard;
  (void)fd;
  (void)format;
  (void)size;
}

static void keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, struct wl_surface *wl_surface,
                           struct wl_array *keys) {
  (void)data;
  (void)wl_keyboard;
  (void)keys;
  (void)wl_surface;
  (void)serial;
}

static void keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, struct wl_surface *wl_surface) {
  (void)data;
  (void)wl_keyboard;
  (void)wl_surface;
  (void)serial;
}

static void keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                         uint32_t serial, uint32_t time, uint32_t key,
                         uint32_t state) {
  (void)data;
  (void)wl_keyboard;
  (void)serial;
  (void)time;

  assert((state == 1 || state == 0) && "unrecognized key state");
  shl_kbd_event_t evt =
      state == 1 ? SHL_KBD_EVENT_KEYDOWN : SHL_KBD_EVENT_KEYUP;

  shl_key_t evt_key;
  switch (key) {
  case 58: // escape
    evt_key = SHL_KEY_CANCEL;
    break;
  case 108: // arrow up
    evt_key = SHL_KEY_UP;
    break;
  case 103: // arrow down
    evt_key = SHL_KEY_DOWN;
    break;
  case 105: // arrow left
    evt_key = SHL_KEY_LEFT;
    break;
  case 106: // arrow right
    evt_key = SHL_KEY_RIGHT;
    break;
  case 28: // enter
    evt_key = SHL_KEY_CONFIRM;
    break;
  default:
    evt_key = SHL_KEY_UNKNOWN;
    log_debug("unknown keyboard key %u", key);
  }

  if (shell.callbacks.key(evt, evt_key)) {
    shl_draw();
  }
}
static void keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                               uint32_t serial, uint32_t mods_depressed,
                               uint32_t mods_latched, uint32_t mods_locked,
                               uint32_t group) {
  (void)data;
  (void)wl_keyboard;
  (void)serial;
  (void)mods_depressed;
  (void)mods_latched;
  (void)mods_locked;
  (void)group;
}
static void keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                                 int32_t rate, int32_t delay) {
  (void)data;
  (void)wl_keyboard;
  log_debug("keyboard repeat info event: rate=%d delay=%d", rate, delay);
}

static void pointer_enter(void *data, struct wl_pointer *wl_pointer,
                          uint32_t serial, struct wl_surface *s,
                          wl_fixed_t surface_x, wl_fixed_t surface_y) {
  (void)data;
  (void)wl_pointer;
  (void)s;
  pointer_x = wl_fixed_to_double(surface_x);
  pointer_y = wl_fixed_to_double(surface_y);

  pointer_serial = serial;
  shl_set_cursor(SHL_CURSOR_DEFAULT, true);
}

static void pointer_leave(void *data, struct wl_pointer *wl_pointer,
                          uint32_t serial, struct wl_surface *s) {
  (void)data;
  (void)wl_pointer;
  (void)serial;
  (void)s;
}

static void pointer_motion(void *data, struct wl_pointer *wl_pointer,
                           uint32_t time, wl_fixed_t surface_x,
                           wl_fixed_t surface_y) {
  (void)data;
  (void)wl_pointer;
  (void)time;
  pointer_x = wl_fixed_to_double(surface_x);
  pointer_y = wl_fixed_to_double(surface_y);

  if (shell.callbacks.pointer(SHL_PTR_EVENT_MOVE, SHL_PTR_BTN_UNKNOWN,
                              pointer_x, pointer_y)) {
    shl_draw();
  }
}

static void pointer_button(void *data, struct wl_pointer *wl_pointer,
                           uint32_t serial, uint32_t time, uint32_t button,
                           uint32_t state) {
  (void)data;
  (void)wl_pointer;
  (void)serial;
  (void)time;

  assert((state == 1 || state == 0) && "unrecognized button state");
  if (state != 1) {
    // act on press, as we do for keys
    return;
  }

  shl_ptr_btn_t evt_btn;
  switch (button) {
  case 0x110: // left mouse button
    evt_btn = SHL_PTR_BTN_LEFT;
    break;
  default:
    evt_btn = SHL_PTR_BTN_UNKNOWN;
    log_debug("unknown pointer button %u", button);
  }

  if (shell.callbacks.pointer(SHL_PTR_EVENT_CLICK, evt_btn, pointer_x,
                              pointer_y)) {
    shl_draw();
  }
}

static void pointer_axis(void *data, struct wl_pointer *wl_pointer,
                         uint32_t time, uint32_t axis, wl_fixed_t value) {
  (void)data;
  (void)wl_pointer;
  (void)time;
  (void)axis;
  (void)value;
}

static void pointer_frame(void *data, struct wl_pointer *wl_pointer) {
  (void)data;
  (void)wl_pointer;
}

static void pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                                uint32_t axis_source) {
  (void)data;
  (void)wl_pointer;
  (void)axis_source;
}

static void pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                              uint32_t time, uint32_t axis) {
  (void)data;
  (void)wl_pointer;
  (void)time;
  (void)axis;
}

static void pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
                                  uint32_t axis, int32_t discrete) {
  (void)data;
  (void)wl_pointer;
  (void)axis;
  (void)discrete;
}

result_t shl_create(shl_callbacks_t cbs, shl_shell_t **shl) {
  assert(shl && "destination shell must be provided");
  assert(cbs.draw && "draw callback must be non-null");
  assert(cbs.key && "key callback must be non-null");
  assert(cbs.pointer && "pointer callback must be non-null");

  display = wl_display_connect(NULL);
  if (!display) {
    log_error("failed to connect to Wayland display", NULL);
    return ERR_SHL_WAYLAND;
  }

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display); // wait until the global handler has returned

  if (!compositor || !shm || !layer_shell || !seat) {
    log_error("missing required global", NULL);
    return ERR_SHL_WAYLAND;
  }

  shell.shm = shm;
  shell.callbacks = cbs;

  wl_seat_add_listener(seat, &seat_listener, NULL);
  wl_display_roundtrip(display); // wait for input to be bound

  surface = wl_compositor_create_surface(compositor);
  layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      layer_shell, surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "dewsesh");

  // anchoring to every edge with a zero size lets the compositor pick the
  // size, i.e. the surface fills all available space on its output
  zwlr_layer_surface_v1_set_anchor(layer_surface,
                                   ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                                       ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT);
  zwlr_layer_surface_v1_set_size(layer_surface, 0, 0);
  zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, -1);
  zwlr_layer_surface_v1_set_keyboard_interactivity(
      layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE);

  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                     NULL);
  wl_surface_commit(surface);

  *shl = &shell;
  return OK;
}

void shl_draw(void) {
  ctx_t *c = NULL;
  shell.callbacks.draw(surface_width, surface_height, &c);
  if (!c)
    return;

  // FIXME: check+log draw errors via a ctx_status() call instead of
  // leaking cairo into shell.c (also reconsider the "ctx" name)

  wl_surface_attach(surface, c->wl.buffer, 0, 0);
  wl_surface_damage_buffer(surface, 0, 0, (int)c->width, (int)c->height);
  wl_surface_commit(surface);
}

void shl_set_cursor(shl_cursor_t c, bool force) {
  if (!cursor_shape_device || pointer_serial == 0)
    return;

  if (!force && c == cursor)
    return;

  uint32_t shape = c == SHL_CURSOR_POINTER
                       ? WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_POINTER
                       : WP_CURSOR_SHAPE_DEVICE_V1_SHAPE_DEFAULT;
  wp_cursor_shape_device_v1_set_shape(cursor_shape_device, pointer_serial,
                                      shape);

  cursor = c;
}

result_t shl_run(void) {
  int fd = wl_display_get_fd(display);

  while (true) {
    if (wl_display_flush(display) == -1) {
      log_error("wl_display_flush failed: %s", strerror(errno));
      return ERR_SHL_WAYLAND;
    }

    struct pollfd pfd = {.fd = fd, .events = POLLIN};

    uint64_t ms;
    int deadline = tmr_next(&ms) == OK ? (int)ms : INT_MAX;

    int res = poll(&pfd, 1, deadline);
    if (res < 0) {
      if (errno == EINTR)
        continue;
      log_error("poll failed: %s", strerror(errno));
      return ERR_SHL_POLL;
    }

    if (res > 0 && (pfd.revents & POLLIN) &&
        wl_display_dispatch(display) == -1) {
      log_error("wl_display_dispatch failed: %s", strerror(errno));
      return ERR_SHL_WAYLAND;
    }

    if (res == 0 && tmr_fire()) {
      shl_draw();
    }
  }

  return OK;
}
