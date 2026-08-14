#include "result.h"

const char *RESULT_MESSAGE[RESULTS] = {
#define X(name, str) [name] = str,
    RESULT_LIST
#undef X
};
