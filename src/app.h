#pragma once

#define APP_OPTIONS_X                                                          \
  X(LOCK, "Lock", "")                                                       \
  X(SUSPEND, "Suspend", "")                                                 \
  X(HIBERNATE, "Hibernate", "")                                             \
  X(RESTART, "Restart", "")                                                 \
  X(SHUTDOWN, "Shutdown", "")

#define X(name, label, icon) APP_OPTION_##name,
typedef enum { APP_OPTIONS_X APP_OPTIONS } app_option_t;
#undef X

#define X(name, label, icon) label,
const char *APP_OPTION_LABEL[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, icon) icon,
const char *APP_OPTION_ICON[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

typedef struct {
  app_option_t option;
} app_state_t;
