#pragma once

#include "timer.h"
#include <stdint.h>
typedef enum {
  APP_STATUS_PRISTINE,
  APP_STATUS_ERRORED,
  APP_STATUS_INHIBIT,
} app_status_t;

#define APP_OPTIONS_X                                                          \
  X(LOCK, "Lock", "Locking", "", "The system will lock in 10s")             \
  X(SUSPEND, "Suspend", "Suspending", "", "The system will suspend in 10s") \
  X(HIBERNATE, "Hibernate", "Hibernating", "",                              \
    "The system will hibernate in 10s")                                        \
  X(LOGOUT, "Logout", "Logging out", "",                                    \
    "The system will log you out in 10s")                                      \
  X(RESTART, "Restart", "Restarting", "", "The system will restart in 10s") \
  X(SHUTDOWN, "Shutdown", "Shutting down", "",                              \
    "The system will shut down in 10s")

#define X(name, label, msg, icon, countdown) APP_OPTION_##name,
typedef enum { APP_OPTIONS_X APP_OPTIONS } app_option_t;
#undef X

extern const char *APP_OPTION_LABEL[APP_OPTIONS];
extern const char *APP_OPTION_ICON[APP_OPTIONS];
extern const char *APP_OPTION_MSG[APP_OPTIONS];
extern const char *APP_OPTION_COUNTDOWN[APP_OPTIONS];

typedef struct {
  app_option_t option;
  app_status_t status;
  tmr_timer_t *action;
} app_state_t;
