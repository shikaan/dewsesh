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
#include <signal.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wlr-layer-shell-unstable-v1.h>

void cairo_set_source_u32(cairo_t *cairo, uint32_t color) {
	cairo_set_source_rgba(cairo,
			(color >> (3*8) & 0xFF) / 255.0,
			(color >> (2*8) & 0xFF) / 255.0,
			(color >> (1*8) & 0xFF) / 255.0,
			(color >> (0*8) & 0xFF) / 255.0);
}

static void draw_frame(ctx_t **ctx) {
  result_t r = ctx_get(640, 480, ctx);
  if (r != OK) {
    return;
  }

  cairo_t *c = (*ctx)->cairo.ctx;
  cairo_set_operator(c, CAIRO_OPERATOR_SOURCE);
  cairo_set_source_u32(c, 0x00000000);
  cairo_paint(c);

  cairo_set_operator(c, CAIRO_OPERATOR_OVER);
  cairo_set_source_u32(c, 0xffffffff);
  cairo_rectangle(c, 0, 0, (int)(*ctx)->width, (int)(*ctx)->height);
  cairo_fill(c);
}

callbacks_t callbacks = {
    .draw = draw_frame,
};

int main(void) {
  log_init(LOG_LEVEL_DEBUG);

  shell_t *shl = NULL;
  shl_init(640, 480, callbacks, &shl);
  ctx_init(640, 480, shl->shm);

  // FIXME: Autoclose the app in 5s, to prevent the pkill dance. 
  signal(SIGALRM, exit);
  alarm(5);

  shl_run();

  return 0;
}
