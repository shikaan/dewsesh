#include "src/ctx.h"
#include "src/log.h"
#include "src/result.h"
#include "src/shell.h"
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

static void draw_frame(ctx_t **ctx) {
  result_t r = ctx_get(400, 200, ctx);
  if (r != OK) {
    return;
  }

  cairo_t *cr = (*ctx)->cairo.ctx;
  // Fully transparent background.
  cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_rgba(cr, 0, 0, 0, 0);
  cairo_paint(cr);

  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  cairo_set_source_rgba(cr, 0.12, 0.12, 0.15, 0.95);
  double margin = 20;
  cairo_rectangle(cr, margin, margin, (int)(*ctx)->width, (int)(*ctx)->height);
  cairo_fill(cr);
}

callbacks_t callbacks = {
    .draw = draw_frame,
};

int main(void) {
  log_init(LOG_LEVEL_DEBUG);

  shell_t *shl = NULL;
  shl_init(400, 200, callbacks, &shl);
  ctx_init(400, 200, shl->shm);

  shl_run();

  return 0;
}
