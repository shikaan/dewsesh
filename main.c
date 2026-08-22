#include "src/app.h"
#include "src/cli.h"
#include "src/config.h"
#include "src/ctx.h"
#include "src/log.h"
#include "src/result.h"
#include "src/shell.h"
#include "src/spawn.h"
#include "src/timer.h"
#include "src/ui.h"
#include <assert.h>
#include <cairo/cairo.h>
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

#ifdef __SANITIZE_ADDRESS__
// fontconfig caches its config/font data for the life of the process and
// never frees it unless the app calls FcFini() which cairo doesn't do
const char *__lsan_default_suppressions(void) {
  return "leak:libfontconfig.so\n";
}
#endif

static app_state_t state = {
    .option = APP_OPTION_LOCK,
    .status = APP_STATUS_PRISTINE,
    .action = NULL,
};
enum {
  BUTTONS_PER_ROW = 3,
  BUTTON_ROWS = APP_OPTIONS / BUTTONS_PER_ROW,
};

typedef struct {
  double x, y, w, h;
} rect_t;
static rect_t btn_hit_targets[APP_OPTIONS];

static config_t *config = NULL;

static void handle_draw(uint32_t w, uint32_t h, ctx_t **ctx) {
  static char msg[128];
  static char name[32];
  static char host[32];

  result_t r = ctx_get(w, h, ctx);
  if (r != OK) {
    return;
  }

  memset(btn_hit_targets, 0, sizeof(btn_hit_targets));

  ui_init(*ctx, config->color.overlay);
  const double vspace = config->font.size * 1.5;
  const double hspace = config->font.size * 2.5;

  const double btnh = config->font.size * 5.5;
  double btnw = config->font.size * 9;
  double btnpady = config->font.size * 3;
  double btnpadx = config->font.size * 4;
  double btnboxh = btnh + btnpady;
  double btnboxw = btnw + btnpadx;
  double nbtnrows = (double)BUTTON_ROWS;

  double framew = (double)BUTTONS_PER_ROW * btnboxw - btnpadx;
  double framex = (double)w / 2 - framew / 2;

  double btnx = framex;

  double headerh = config->font.size * 3;
  double headery = vspace;

  double footerh = config->font.size * 3;
  double footery_relative = btnboxh * nbtnrows + vspace + headerh;

  double frameh = footery_relative - btnpady + footerh + vspace;
  double framey = (double)h / 2 - frameh / 2;
  double btny = framey + headery + headerh;

  ui_rect(framex - hspace, framey - vspace, framew + 2 * hspace,
          frameh + 2 * vspace, config->color.window);

  ui_txt_t txt_opts = {
      .color = config->color.text,
      .size = config->font.size,
      .family = config->font.status,
      .weight = UI_TXT_WEIGHT_BOLD,
      .align = UI_TXT_ALIGN_CENTER,
  };
  gethostname(host, sizeof(host));
  getlogin_r(name, sizeof(name));
  snprintf(msg, sizeof(msg), "%s@%s", name, host);
  ui_txt(txt_opts, framex + framew / 2, framey + vspace, msg);

  if (state.status == APP_STATUS_INHIBIT) {
    txt_opts.size = config->font.size * 3;
    txt_opts.family = config->font.text;
    txt_opts.weight = UI_TXT_WEIGHT_BOLD;
    txt_opts.align = UI_TXT_ALIGN_CENTER;

    sprintf(msg, "%s...", APP_OPTION_MSG[state.option]);

    ui_txt_t sub_opts = txt_opts;
    sub_opts.size = config->font.size;
    sub_opts.weight = UI_TXT_WEIGHT_NORMAL;
    const char *submsg = APP_OPTION_COUNTDOWN[state.option];

    ui_txt_bounds_t msg_bounds, sub_bounds;
    ui_txt_init(txt_opts, msg, &msg_bounds);
    ui_txt_init(sub_opts, submsg, &sub_bounds);

    double gap = config->font.size * 1.5;
    double gridh = nbtnrows * btnboxh - btnpady;
    double content_height = msg_bounds.height + gap + sub_bounds.height;
    double top = btny + (gridh - content_height) / 2;
    double msgy = top - msg_bounds.y_bearing;
    double suby = top + msg_bounds.height + gap - sub_bounds.y_bearing;

    ui_txt_commit(sub_opts, framex + framew / 2, suby, &sub_bounds, submsg);
    ui_txt_init(txt_opts, msg, &msg_bounds);
    ui_txt_commit(txt_opts, framex + framew / 2, msgy, &msg_bounds, msg);
  } else {
    ui_btn_t btn_opts = {
        .icon_family = config->font.icon,
        .text_family = config->font.text,
        .size = config->font.size,
        .color =
            {
                [UI_BTN_STATUS_NONE] =
                    {
                        .bg = config->color.button,
                        .fg = config->color.text,
                    },
                [UI_BTN_STATUS_SELECTED] =
                    {
                        .bg = config->color.selected,
                        .fg = config->color.text,
                    },
            },
    };

    for (int i = 0; i < APP_OPTIONS; i++) {
      double x = btnx + btnboxw * (i % BUTTONS_PER_ROW);
      double y = btny + btnboxh * (i >= BUTTONS_PER_ROW);

      app_option_t option = (app_option_t)i;

      const char *label = APP_OPTION_LABEL[option];
      const char *icon = APP_OPTION_ICON[option];

      ui_btn_status_t btn_status = UI_BTN_STATUS_NONE;
      if (option == state.option) {
        btn_status = UI_BTN_STATUS_SELECTED;
      }

      ui_btn(btn_opts, x, y, btnw, btnh, label, icon, btn_status);
      btn_hit_targets[i] = (rect_t){.x = x, .y = y, .w = btnw, .h = btnh};
    }
  }

  txt_opts.size = config->font.size;
  txt_opts.align = UI_TXT_ALIGN_CENTER;

  const char *status = NULL;
  if (state.status == APP_STATUS_ERRORED) {
    txt_opts.family = config->font.text;
    txt_opts.color = config->color.error;
    txt_opts.weight = UI_TXT_WEIGHT_BOLD;

    sprintf(msg, "%s failed. See logs for details.",
            APP_OPTION_LABEL[state.option]);
    status = msg;
  } else if (state.status == APP_STATUS_INHIBIT) {
    status = "ENTER Confirm · ESC Cancel";
    txt_opts.family = config->font.status;
    txt_opts.color = config->color.status;
    txt_opts.weight = UI_TXT_WEIGHT_NORMAL;
  } else {
    status = "ARROWS Move · ENTER Confirm · ESC Cancel";
    txt_opts.family = config->font.status;
    txt_opts.color = config->color.status;
    txt_opts.weight = UI_TXT_WEIGHT_NORMAL;
  }
  assert(status && "status must be defined");

  ui_txt(txt_opts, framex + framew / 2,
         framey + footery_relative + config->font.size, status);
}

static bool action(void *data) {
  log_debug("launching option %d", state.option);
  (void)data;

  if (spw_launch(config->actions[state.option]) == OK)
    exit(0);

  state.status = APP_STATUS_ERRORED;
  return true;
}

static bool confirm(void) {
  if (state.action && state.status == APP_STATUS_INHIBIT) {
    tmr_cancel(state.action);
    state.action = NULL;
    return action(NULL);
  }

  bool requires_inhibition = state.option == APP_OPTION_RESTART ||
                             state.option == APP_OPTION_SHUTDOWN ||
                             state.option == APP_OPTION_LOGOUT;
  if (requires_inhibition) {
    // FIXME: should we lock the state here?
    tmr_timeout(10000, action, NULL, &state.action);
    state.status = APP_STATUS_INHIBIT;
    return true;
  }

  return action(NULL);
}

static bool handle_key(shl_kbd_event_t evt, shl_key_t key) {
  if (key == SHL_KEY_UNKNOWN || evt == SHL_KBD_EVENT_KEYUP)
    return false;

  log_debug("received event %d, key %d", evt, key);

  if (evt == SHL_KBD_EVENT_KEYDOWN) {
    if (key == SHL_KEY_CANCEL) {
      if (state.action && state.status == APP_STATUS_INHIBIT) {
        tmr_cancel(state.action);
        state.action = NULL;
        state.status = APP_STATUS_PRISTINE;
        return true;
      } else {
        exit(0);
      }
    }

    int delta = 0;
    if (key == SHL_KEY_DOWN)
      delta = -BUTTONS_PER_ROW;
    else if (key == SHL_KEY_UP)
      delta = BUTTONS_PER_ROW;
    else if (key == SHL_KEY_LEFT)
      delta = -1;
    else if (key == SHL_KEY_RIGHT)
      delta = 1;

    if (delta != 0) {
      // Reset status on movement
      state.status = APP_STATUS_PRISTINE;
      int opt = (int)state.option + delta;
      if (opt >= 0 && opt < APP_OPTIONS)
        state.option = (app_option_t)opt;
      return true;
    }

    if (key == SHL_KEY_CONFIRM) {
      return confirm();
    }
  }

  log_debug("unhandled keyboard event %d, key %d", evt, key);
  return false;
}

static int option_at(double x, double y) {
  for (int i = 0; i < APP_OPTIONS; i++) {
    rect_t rect = btn_hit_targets[i];
    if (rect.w <= 0 || rect.h <= 0)
      continue; // no button drawn here

    if (x < rect.x || x >= rect.x + rect.w)
      continue;
    if (y < rect.y || y >= rect.y + rect.h)
      continue;

    return i;
  }

  return -1;
}

static bool handle_pointer(shl_ptr_event_t evt, shl_ptr_btn_t btn, double x,
                           double y) {
  if (state.status == APP_STATUS_INHIBIT) {
    shl_set_cursor(SHL_CURSOR_DEFAULT, false);
    return false;
  }

  int option = option_at(x, y);

  if (evt == SHL_PTR_EVENT_MOVE) {
    shl_set_cursor(option < 0 ? SHL_CURSOR_DEFAULT : SHL_CURSOR_POINTER, false);

    if (option < 0 || (app_option_t)option == state.option)
      return false;

    state.option = (app_option_t)option;
    state.status = APP_STATUS_PRISTINE;
    return true;
  }

  if (btn != SHL_PTR_BTN_LEFT)
    return false;

  log_debug("received click at %.0fx%.0f", x, y);

  // Close when clicking on a blank spot
  if (option < 0)
    exit(0);

  state.option = (app_option_t)option;
  state.status = APP_STATUS_PRISTINE;
  return confirm();
}

static shl_callbacks_t callbacks = {
    .draw = handle_draw,
    .key = handle_key,
    .pointer = handle_pointer,
};

int main(int argc, char *const *argv) {
  cli_opts_t *cli_opts;
  cli_parse(argc, argv, &cli_opts);
  assert(cli_opts && "cli_opts must be non-null");

  log_init(cli_opts->debug ? LOG_LEVEL_DEBUG : LOG_LEVEL_INFO);
  log_debug("cli options: configuration = '%s'",
            cli_opts->config ? cli_opts->config : "(nil)");
  log_debug("cli options: debug = %s", cli_opts->debug ? "true" : "false");

  const char *config_path = cli_opts->config ? cli_opts->config : cfg_path();
  cfg_read(config_path, &config);
  assert(config && "config must be non-null");

  shl_shell_t *shl = NULL;
  if (shl_create(callbacks, &shl) != OK)
    return 1;

  assert(shl && "must be non-null");
  return shl_run() == OK ? 0 : 1;
}
