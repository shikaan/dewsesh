#pragma once

typedef enum {
  OK = 0,
  ERROR = -1,

  ERR_CTX = -100,
  ERR_CTX_ALLOCATION = -101,
  ERR_CTX_CAIRO = -102
} result_t;

