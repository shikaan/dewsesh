#include "src/ctx.h"
#include "src/log.h"
#include <cairo/cairo.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wlr-layer-shell-unstable-v1.h>

static struct wl_compositor *compositor = NULL;
static struct wl_shm *shm = NULL;
static struct zwlr_layer_shell_v1 *layer_shell = NULL;

static struct wl_surface *surface;
static struct zwlr_layer_surface_v1 *layer_surface;

static uint32_t width = 400, height = 200;

// FIXME: fish from the pool
static ctx_t ctx;

static struct wl_buffer *draw_frame(void) {
  // FIXME: fish from the pool
  ctx_create(&ctx, 400, 200, shm);

  cairo_t *cr = ctx.cairo.ctx;
  // Fully transparent background.
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_paint(cr);

  // The "box": a rounded-ish filled rectangle in the middle of our surface.
  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  cairo_set_source_rgba(cr, 0.12, 0.12, 0.15, 0.95);
  double margin = 20;
  cairo_rectangle(cr, margin, margin, width - 2 * margin, height - 2 * margin);
  cairo_fill(cr);

  return ctx.wl.buffer;
}

// --- layer surface events ---

static void layer_surface_configure(void *data,
                                    struct zwlr_layer_surface_v1 *surf,
                                    uint32_t serial, uint32_t w, uint32_t h) {
  (void)data;
  log_debug("configure event: w=%u h=%u", w, h);
  if (w > 0)
    width = w;
  if (h > 0)
    height = h;
  zwlr_layer_surface_v1_ack_configure(surf, serial);

  struct wl_buffer *buffer = draw_frame();
  wl_surface_attach(surface, buffer, 0, 0);
  wl_surface_damage_buffer(surface, 0, 0, (int)width, (int)height);
  wl_surface_commit(surface);
}

static void layer_surface_closed(void *data,
                                 struct zwlr_layer_surface_v1 *surf) {
  (void)data;
  (void)surf;
  exit(0);
}

static const struct zwlr_layer_surface_v1_listener layer_surface_listener = {
    .configure = layer_surface_configure,
    .closed = layer_surface_closed,
};

// --- registry: bind the globals we need ---

static void registry_global(void *data, struct wl_registry *registry,
                            uint32_t name, const char *interface,
                            uint32_t version) {
  (void)data;
  (void)version;

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

static const struct wl_registry_listener registry_listener = {
    .global = registry_global,
    .global_remove = registry_global_remove,
};

int main(void) {
  log_init(LOG_LEVEL_DEBUG);

  struct wl_display *display = wl_display_connect(NULL);
  if (!display) {
    log_error("failed to connecto to Wayland display", NULL);
    return 1;
  }

  struct wl_registry *registry = wl_display_get_registry(display);
  wl_registry_add_listener(registry, &registry_listener, NULL);
  wl_display_roundtrip(display); // wait until the global handler has returned

  if (!compositor || !shm || !layer_shell) {
    log_error("missing required global", NULL);
    return 1;
  }

  surface = wl_compositor_create_surface(compositor);

  // NULL output = let the compositor pick (usually focused output).
  // Layer: OVERLAY so we render above panels/normal windows, like a menu.
  layer_surface = zwlr_layer_shell_v1_get_layer_surface(
      layer_shell, surface, NULL, ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY,
      "layer-demo");

  // Don't anchor to any edge -> compositor centers us at our requested size.
  zwlr_layer_surface_v1_set_size(layer_surface, width, height);
  zwlr_layer_surface_v1_set_keyboard_interactivity(
      layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND);

  zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener,
                                     NULL);
  wl_surface_commit(surface);

  while (wl_display_dispatch(display) != -1) {
    // event loop; configure event above does the actual drawing
  }

  return 0;
}
