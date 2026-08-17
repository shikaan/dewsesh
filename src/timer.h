#pragma once
#include "result.h"
#include <stddef.h>
#include <stdint.h>

typedef struct {
  void (*cb)(void *);
  void *data;
  uint64_t exp;
} tmr_timer_t;

result_t tmr_timeout(uint64_t ms, void (*cb)(void *data), void *data,
                     tmr_timer_t **timer);
result_t tmr_cancel(tmr_timer_t *timer);

result_t tmr_next(uint64_t *ms);
void tmr_fire(void);
