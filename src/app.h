#pragma once

#define APP_OPTIONS_X                                                          \
  X(LOCK, "Lock", "", "dewlock")                                            \
  X(SUSPEND, "Suspend", "", "systemctl suspend-then-hibernate")             \
  X(HIBERNATE, "Hibernate", "", "systemctl hibernate")                      \
  X(RESTART, "Restart", "", "systemctl reboot")                             \
  X(SHUTDOWN, "Shutdown", "", "systemctl poweroff")

#define X(name, label, icon, cmd) APP_OPTION_##name,
typedef enum { APP_OPTIONS_X APP_OPTIONS } app_option_t;
#undef X

#define X(name, label, icon, cmd) label,
const char *APP_OPTION_LABEL[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, icon, cmd) icon,
const char *APP_OPTION_ICON[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, icon, cmd) cmd,
const char *APP_OPTION_CMD[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

typedef struct {
  app_option_t option;
} app_state_t;
