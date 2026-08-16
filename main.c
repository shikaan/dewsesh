#include "src/ctx.h"
#include "src/log.h"
#include "src/result.h"
#include "src/shell.h"
#include "src/ui.h"
#include <cairo/cairo.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
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

  ui_init(*ctx, 0x00000099);
  double vspace = 24;
  double hspace = 36;
  double nbuttons = 5;

  double framew = 400;
  double framex = (double)w / 2 - framew / 2;

  double btnh = 48;
  double btnw = framew - hspace * 2;
  double btnx = framex + hspace;
  double pady = 16;
  double btnboxh = btnh + pady;

  double headerh = 80;
  double headery = vspace;

  double footerh = 48;
  double footerrely = btnboxh * nbuttons + vspace + headerh;

  double frameh = footerrely - pady + footerh + vspace;
  double framey = (double)h / 2 - frameh / 2;
  double buttony = framey + headery + headerh;

  ui_text_t opts = {
      .color = 0xeaeaeaff,
      .size = 28,
      .family = "Noto Sans",
      .weight = CAIRO_FONT_WEIGHT_BOLD,
  };
  ui_text(framex + hspace, framey + vspace + 24, opts, "End Session");
  opts.size = 14;
  opts.weight = CAIRO_FONT_WEIGHT_NORMAL;
  ui_text(framex + hspace, framey + vspace + 48, opts, "Select an option");

  ui_btn_t btn_opts = {
      .icon_family = "FontAwesome",
      .text_family = "Noto Sans",
      .color =
          {
              [UI_BTN_STATUS_NONE] = {.bg = 0x00000000, .fg = 0xeaeaeaff},
              [UI_BTN_STATUS_SELECTED] = {.bg = 0x82a2beff, .fg = 0xeaeaeaff},
          },
  };

  ui_btn(btnx, buttony + btnboxh * 0, btnw, btnh, btn_opts, "Lock", "",
         UI_BTN_STATUS_SELECTED);
  ui_btn(btnx, buttony + btnboxh * 1, btnw, btnh, btn_opts, "Suspend", "",
         UI_BTN_STATUS_NONE);
  ui_btn(btnx, buttony + btnboxh * 2, btnw, btnh, btn_opts, "Hibernate", "",
         UI_BTN_STATUS_NONE);
  ui_btn(btnx, buttony + btnboxh * 3, btnw, btnh, btn_opts, "Restart", "",
         UI_BTN_STATUS_NONE);
  ui_btn(btnx, buttony + btnboxh * 4, btnw, btnh, btn_opts, "Shutdown", "",
         UI_BTN_STATUS_NONE);

  ui_text_bounds_t bounds;
  opts.size = 12;
  const char *footer = "Arrows to move · Enter to select · Esc to cancel";
  ui_text_init(opts, footer, &bounds);
  ui_text_commit(framex + framew / 2 - bounds.width / 2,
                 framey + footerrely + 8, footer);
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
