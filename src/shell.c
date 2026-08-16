#include "shell.h"
#include "ctx.h"
#include "log.h"
#include "result.h"
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client-protocol.h>
#include <wayland-client.h>
#include <wlr-layer-shell-unstable-v1.h>

static shell_t shell = {0};

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

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct zwlr_layer_shell_v1 *layer_shell = NULL;
static struct wl_seat *seat = NULL;

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
  if (capabilities & WL_SEAT_CAPABILITY_KEYBOARD) {
    struct wl_keyboard *kbd = wl_seat_get_keyboard(wl_seat);
    wl_keyboard_add_listener(kbd, &keyboard_listener, NULL);
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
  log_debug("keyboard keymap event: format=%u size=%u", format, size);
}
static void keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, struct wl_surface *wl_surface,
                           struct wl_array *keys) {
  (void)data;
  (void)wl_keyboard;
  (void)keys;
  (void)wl_surface;
  log_debug("keyboard enter event: serial=%u", serial);
}
static void keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                           uint32_t serial, struct wl_surface *wl_surface) {
  (void)data;
  (void)wl_keyboard;
  (void)wl_surface;
  log_debug("keyboard leave event: serial=%u", serial);
}
static void keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                         uint32_t serial, uint32_t time, uint32_t key,
                         uint32_t _key_state) {
  (void)data;
  (void)wl_keyboard;
  log_debug("keyboard key event: serial=%u time=%u key=%u state=%u", serial,
            time, key, _key_state);
}
static void keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                               uint32_t serial, uint32_t mods_depressed,
                               uint32_t mods_latched, uint32_t mods_locked,
                               uint32_t group) {
  (void)data;
  (void)wl_keyboard;
  log_debug("keyboard modifiers event: serial=%u depressed=%u latched=%u "
            "locked=%u group=%u",
            serial, mods_depressed, mods_latched, mods_locked, group);
}
static void keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                                 int32_t rate, int32_t delay) {
  (void)data;
  (void)wl_keyboard;
  log_debug("keyboard repeat info event: rate=%d delay=%d", rate, delay);
}

result_t shl_init(callbacks_t cbs, shell_t **shl) {
  assert(shl && "destination shell must be provided");
  assert(cbs.draw && "draw callback must be non-null");

  display = wl_display_connect(NULL);
  if (!display) {
    log_error("failed to connecto to Wayland display", NULL);
    return ERR_SHL_WAYLAND;
  }

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display); // wait until the global handler has returned

  if (!compositor || !shm || !layer_shell) {
    log_error("missing required global", NULL);
    return ERR_SHL_WAYLAND;
  }

  shell.shm = shm;
  shell.callbacks = cbs;

  wl_seat_add_listener(seat, &seat_listener, NULL);

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
  zwlr_layer_surface_v1_set_keyboard_interactivity(
      layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_EXCLUSIVE);

  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                     NULL);
  wl_surface_commit(surface);

  log_debug("commit", NULL);
  *shl = &shell;
  return OK;
}

void shl_draw(void) {
  ctx_t *c = NULL;
  shell.callbacks.draw(surface_width, surface_height, &c);
  if (!c)
    return;

  wl_surface_attach(surface, c->wl.buffer, 0, 0);
  wl_surface_damage_buffer(surface, 0, 0, (int)c->width, (int)c->height);
  wl_surface_commit(surface);
}

void shl_run(void) {
  while (wl_display_dispatch(display) != -1) {
    // event loop; configure event above does the actual drawing
  }
}
