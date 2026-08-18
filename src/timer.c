#include "timer.h"
#include "log.h"
#include "result.h"
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

enum { MAX_TIMERS = 16 };

static tmr_timer_t timers[MAX_TIMERS] = {0};
static size_t ntimers = 0;

static uint64_t now(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

result_t tmr_timeout(uint64_t ms, tmr_callback_t callback, void *data,
                     tmr_timer_t **timer) {
  assert(callback && "callback must be defined");
  assert(ms > 0 && "ms must be non-zero");

  for (size_t i = 0; i < MAX_TIMERS; i++) {
    if (timers[i].callback == NULL) {
      log_debug("scheduling timer %zu in %llu ms", i, ms);
      timers[i].callback = callback;
      timers[i].data = data;
      timers[i].expiry = now() + ms;
      ntimers++;
      *timer = &timers[i];
      return OK;
    }
  }

  log_error("cannot register timer. Timers list is full %d", MAX_TIMERS);
  return ERR_TMR_TOO_MANY_TIMERS;
}

result_t tmr_cancel(tmr_timer_t *timer) {
  for (size_t i = 0; i < MAX_TIMERS; i++) {
    if (&timers[i] == timer && timer->callback) {
      timer->callback = NULL;
      timer->data = NULL;
      timer->expiry = 0;
      ntimers--;
      return OK;
    }
  }

  log_error("cannot cancel unregistered timer", NULL);
  return ERR_TMR_NOT_FOUND;
}
result_t tmr_next(uint64_t *ms) {
  if (ntimers == 0) {
    *ms = 0;
    return ERR_TMR_NO_TIMERS;
  }

  const uint64_t t = now();
  uint64_t min = UINT64_MAX;

  for (size_t i = 0; i < MAX_TIMERS; i++) {
    if (timers[i].callback && timers[i].expiry < min) {
      min = timers[i].expiry;
    }
  }

  *ms = min > t ? min - t : 0;
  log_debug("next timer in %llu ms", *ms);
  return OK;
}

bool tmr_fire(void) {
  if (ntimers == 0)
    return false;

  const uint64_t t = now();
  bool result = false;
  for (size_t i = 0; i < MAX_TIMERS; i++) {
    tmr_timer_t *timer = &timers[i];
    if (timer->callback && timer->expiry <= t) {
      log_debug("firing timer %zu", i);
      bool (*cb)(void *) = timer->callback;
      void *data = timer->data;
      // free the slot before invoking, in case cb re-arms a timer
      timer->callback = NULL;
      ntimers--;
      result = result || cb(data);
    }
  }

  return result;
}
