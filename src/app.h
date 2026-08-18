#pragma once

#include "timer.h"
#include <stdint.h>
typedef enum {
  APP_STATUS_PRISTINE,
  APP_STATUS_ERRORED,
  APP_STATUS_INHIBIT,
} app_status_t;

#define APP_OPTIONS_X                                                          \
  X(LOCK, "Lock", "Locking", "", "dewlock", "The system will lock in 10s.") \
  X(SUSPEND, "Suspend", "Suspending", "",                                   \
    "systemctl suspend-then-hibernate", "The system will suspend in 10s.")     \
  X(HIBERNATE, "Hibernate", "Hibernating", "", "systemctl hibernate",       \
    "The system will hibernate in 10s.")                                       \
  X(LOGOUT, "Logout", "Logging out", "", "loginctl terminate-session",      \
    "The system will log you out in 10s.")                                     \
  X(RESTART, "Restart", "Restarting", "", "systemctl reboot",               \
    "The system will restart in 10s.")                                         \
  X(SHUTDOWN, "Shutdown", "Shutting down", "", "systemctl poweroff",        \
    "The system will shut down in 10s.")

#define X(name, label, msg, icon, cmd, countdown) APP_OPTION_##name,
typedef enum { APP_OPTIONS_X APP_OPTIONS } app_option_t;
#undef X

#define X(name, label, msg, icon, cmd, countdown) label,
const char *APP_OPTION_LABEL[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, msg, icon, cmd, countdown) icon,
const char *APP_OPTION_ICON[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, msg, icon, cmd, countdown) cmd,
const char *APP_OPTION_CMD[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, msg, icon, cmd, countdown) msg,
const char *APP_OPTION_MSG[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, msg, icon, cmd, countdown) countdown,
const char *APP_OPTION_COUNTDOWN[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

typedef struct {
  app_option_t option;
  app_status_t status;
  tmr_timer_t *action;
} app_state_t;
