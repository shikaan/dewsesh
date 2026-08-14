#pragma once

#define RESULT_LIST                                                            \
  X(OK, "ok")                                                                  \
  X(ERROR, "error")                                                            \
  X(ERR_CTX, "unexpected ctx error")                                           \
  X(ERR_CTX_ALLOCATION, "cannot allocate resources")                           \
  X(ERR_CTX_CAIRO, "cannot instantiate cairo context")                         \
  X(ERR_CTX_MISSING_BUFFER, "cannot find available drawing buffer")

typedef enum {
#define X(name, str) name,
  RESULT_LIST
#undef X
      RESULTS
} result_t;

extern const char *RESULT_MESSAGE[RESULTS];
