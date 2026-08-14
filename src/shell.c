#include "shell.h"
#include "ctx.h"
#include "log.h"
#include "result.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <wlr-layer-shell-unstable-v1.h>

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

static struct wl_display *display = NULL;
static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct zwlr_layer_shell_v1 *layer_shell = NULL;

static struct wl_surface *surface;
static struct zwlr_layer_surface_v1 *layer_surface;

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *surf,
                                    uint32_t serial, uint32_t w, uint32_t h) {
  (void)data;
  // FIXME: w,h, can be zero!
  // FIXME: is this where we set the height and width for context?
  log_debug("configure event: w=%u h=%u", w, h);
  zwlr_layer_surface_v1_ack_configure(surf, serial);
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
  }
}

static void registry_global_remove(void *data, struct wl_registry *registry,
                                   uint32_t name) {
  (void)data;
  (void)registry;
  (void)name;
}

static shell_t shell = {0};
result_t shl_init(uint32_t w, uint32_t h, callbacks_t cbs, shell_t **shl) {
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

  surface = wl_compositor_create_surface(compositor);
  layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      layer_shell, surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "dewsesh");

  zwlr_layer_surface_v1_set_size(layer_surface, w, h);
  zwlr_layer_surface_v1_set_keyboard_interactivity(
      layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND);

  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                     NULL);
  wl_surface_commit(surface);

  log_debug("commit", NULL);
  *shl = &shell;
  return OK;
}

void shl_draw(void) {
  ctx_t *c = NULL;
  shell.callbacks.draw(&c);
  wl_surface_attach(surface, c->wl.buffer, 0, 0);
  wl_surface_damage_buffer(surface, 0, 0, (int)c->width, (int)c->height);
  wl_surface_commit(surface);
}

void shl_run(void) {
  while (wl_display_dispatch(display) != -1) {
    // event loop; configure event above does the actual drawing
  }
}
