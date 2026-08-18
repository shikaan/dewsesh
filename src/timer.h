#pragma once
#include "result.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef bool (*tmr_callback_t)(void *);

typedef struct {
  tmr_callback_t callback;
  void *data;
  uint64_t expiry;
} tmr_timer_t;

result_t tmr_timeout(uint64_t ms, tmr_callback_t callback, void *data,
                     tmr_timer_t **timer);
result_t tmr_cancel(tmr_timer_t *timer);

result_t tmr_next(uint64_t *ms);
bool tmr_fire(void);
