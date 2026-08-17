#pragma once

#include <stdint.h>
typedef enum {
  APP_STATUS_PRISTINE,
  APP_STATUS_ERRORED,
  APP_STATUS_INHIBIT,
} app_status_t;

#define APP_OPTIONS_X                                                          \
  X(LOCK, "Lock", "Locking", "", "dewlock", "If you do nothing, the system will lock in 10s.") \
  X(SUSPEND, "Suspend", "Suspending", "", "systemctl suspend-then-hibernate", "If you do nothing, the system will suspend in 10s.") \
  X(HIBERNATE, "Hibernate", "Hibernating", "", "systemctl hibernate", "If you do nothing, the system will hibernate in 10s.") \
  X(LOGOUT, "Logout", "Logging out", "", "loginctl terminate-session", "If you do nothing, the system will log out in 10s.") \
  X(RESTART, "Restart", "Restarting", "", "systemctl reboot", "If you do nothing, the system will restart in 10s.") \
  X(SHUTDOWN, "Shutdown", "Shutting down", "", "systemctl poweroff", "If you do nothing, the system will shut down in 10s.")

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
} app_state_t;
