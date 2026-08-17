#include "src/app.h"
#include "src/ctx.h"
#include "src/log.h"
#include "src/result.h"
#include "src/shell.h"
#include "src/spawn.h"
#include "src/ui.h"
#include <assert.h>
#include <cairo/cairo.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <wayland-client.h>
#include <wlr-layer-shell-unstable-v1.h>

static app_state_t state = {.option = APP_OPTION_LOCK,
                            .status = APP_STATUS_PRISTINE};

static inline int clamp(int x, int min, int max) {
  assert(min < max && "min should be less than max");
  return x < min ? min : (x >= max ? max - 1 : x);
}

static void handle_draw(uint32_t w, uint32_t h, ctx_t **ctx) {
  result_t r = ctx_get(w, h, ctx);
  if (r != OK) {
    return;
  }

  ui_init(*ctx, 0x282c34cd);
  double vspace = 24;

  double btnh = 88;
  double btnw = 144;
  double pady = 48;
  double padx = 64;
  double btnboxh = btnh + pady;
  double btnboxw = btnw + padx;
  double nrows = 2;
  int rowbtns = APP_OPTIONS / (int)nrows;

  double framew = rowbtns * btnboxw - padx;
  double framex = (double)w / 2 - framew / 2;

  double btnx = framex;

  double headerh = 32;
  double headery = vspace;

  double footerh = 48;
  double footery_relative = btnboxh * nrows + vspace + headerh;

  double frameh = footery_relative - pady + footerh + vspace;
  double framey = (double)h / 2 - frameh / 2;
  double btny = framey + headery + headerh;

  ui_txt_t txt_opts = {
      .color = 0xeaeaeaff,
      .size = 16,
      .family = "monospace",
      .weight = CAIRO_FONT_WEIGHT_NORMAL,
      .align = UI_TXT_ALIGN_CENTER,
  };
  ui_txt(txt_opts, framex + framew / 2, framey + vspace, "manuel@debian");

  ui_btn_t btn_opts = {
      .icon_family = "FontAwesome",
      .text_family = "Noto Sans",
      .color =
          {
              [UI_BTN_STATUS_NONE] = {.bg = 0x00000000, .fg = 0xeaeaeaff},
              [UI_BTN_STATUS_SELECTED] = {.bg = 0x82a2be80, .fg = 0xeaeaeaff},
          },
  };

  for (int i = 0; i < APP_OPTIONS; i++) {
    double x = btnx + btnboxw * (i % rowbtns);
    double y = btny + btnboxh * (i >= rowbtns);

    app_option_t option = (app_option_t)i;

    const char *label = APP_OPTION_LABEL[option];
    const char *icon = APP_OPTION_ICON[option];

    ui_btn_status_t btn_status = UI_BTN_STATUS_NONE;
    if (option == state.option) {
      btn_status = UI_BTN_STATUS_SELECTED;
    }

    ui_btn(btn_opts, x, y, btnw, btnh, label, icon, btn_status);
  }

  txt_opts.family = "sans-serif";
  txt_opts.size = 14;
  txt_opts.align = UI_TXT_ALIGN_CENTER;

  const char *status = "TAB Move · ENTER Confirm · ESC Cancel";
  if (state.status == APP_STATUS_ERRORED) {
    txt_opts.color = 0xff6b6bff;
    txt_opts.weight = CAIRO_FONT_WEIGHT_BOLD;

    static char msg[64];
    int l = sprintf(msg, "%s failed. See logs for details.",
                    APP_OPTION_LABEL[state.option]);
    msg[l] = 0;
    status = msg;
  } else {
    txt_opts.color = 0xc4c8c6ff;
    txt_opts.weight = CAIRO_FONT_WEIGHT_NORMAL;
  }

  ui_txt(txt_opts, framex + framew / 2, framey + footery_relative + 16, status);
}

static bool handle_key(shl_kbd_event_t evt, shl_key_t key) {
  log_debug("received event %d, key %d", evt, key);
  if (key == SHL_KEY_UNKNOWN || evt == SHL_KBD_EVENT_KEYUP)
    return false;

  state.status = APP_STATUS_PRISTINE;

  if (key == SHL_KEY_EXIT)
    exit(0);

  if (key == SHL_KEY_DOWN && evt == SHL_KBD_EVENT_KEYDOWN) {
    app_option_t opt =
        (app_option_t)clamp((int)state.option - 1, 0, APP_OPTIONS);
    state.option = opt;
    return true;
  }

  if (key == SHL_KEY_UP && evt == SHL_KBD_EVENT_KEYDOWN) {
    app_option_t opt =
        (app_option_t)clamp((int)state.option + 1, 0, APP_OPTIONS);
    state.option = opt;
    return true;
  }

  if (key == SHL_KEY_SELECT && evt == SHL_KBD_EVENT_KEYDOWN) {
    if (spw_launch(APP_OPTION_CMD[state.option]) == OK)
      exit(0);
    state.status = APP_STATUS_ERRORED;
    return true;
  }

  log_info("unhandled keyboard event %d, key %d", evt, key);
  return false;
}

shl_callbacks_t callbacks = {
    .draw = handle_draw,
    .key = handle_key,
};

int main(void) {
  log_init(LOG_LEVEL_DEBUG);

  shl_shell_t *shl = NULL;
  shl_create(callbacks, &shl);

  shl_run();

  return 0;
}
