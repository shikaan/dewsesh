#include "src/ctx.h"
#include "src/log.h"
#include "src/result.h"
#include "src/shell.h"
#include "src/ui.h"
#include <cairo/cairo.h>
#include <errno.h>
#include <signal.h>
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

static void draw_frame(uint32_t w, uint32_t h, ctx_t **ctx) {
  result_t r = ctx_get(w, h, ctx);
  if (r != OK) {
    return;
  }

  ui_init(*ctx, 0x00000088);
  ui_rect(50, 50, 500, 500, 0x00ff00ff);

  ui_text_t opts = {
      .color = 0xffffffff,
      .size = 16,
      .family = "Noto Sans",
      .weight = CAIRO_FONT_WEIGHT_BOLD,
  };
  ui_text(46, 16, opts, "Suspend");
  ui_text(46, 32, opts, "Hibernate");
  opts.family = "FontAwesome";
  opts.weight = CAIRO_FONT_WEIGHT_NORMAL;
  ui_text(16, 32, opts, " ");
}

callbacks_t callbacks = {
    .draw = draw_frame,
};

int main(void) {
  log_init(LOG_LEVEL_DEBUG);

  shell_t *shl = NULL;
  shl_init(callbacks, &shl);

  // FIXME: Autoclose the app in 5s, to prevent the pkill dance.
  signal(SIGALRM, exit);
  alarm(5);

  shl_run();

  return 0;
}
