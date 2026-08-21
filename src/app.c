#include "app.h"

#define X(name, label, msg, icon, countdown) label,
const char *APP_OPTION_LABEL[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, msg, icon, countdown) icon,
const char *APP_OPTION_ICON[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, msg, icon, countdown) msg,
const char *APP_OPTION_MSG[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X

#define X(name, label, msg, icon, countdown) countdown,
const char *APP_OPTION_COUNTDOWN[APP_OPTIONS] = {APP_OPTIONS_X};
#undef X
