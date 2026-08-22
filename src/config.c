#include "config.h"
#include "cli.h"
#include "color.h"
#include "log.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <wordexp.h>

#define CONFIG_VALUE_SEPARATOR '='
#define CONFIG_NAMESPACE_SEPARATOR '.'
#define CONFIG_COMMENT '#'

#define CONFIG_NAMESPACE_FONT "font"
#define CONFIG_FONT_TEXT "text"
#define CONFIG_FONT_INFO "info"
#define CONFIG_FONT_ICON "icon"
#define CONFIG_FONT_SIZE "size"

#define CONFIG_NAMESPACE_ACTIONS "actions"
#define CONFIG_ACTIONS_LOCK "lock"
#define CONFIG_ACTIONS_SUSPEND "suspend"
#define CONFIG_ACTIONS_HIBERNATE "hibernate"
#define CONFIG_ACTIONS_LOGOUT "logout"
#define CONFIG_ACTIONS_REBOOT "reboot"
#define CONFIG_ACTIONS_SHUTDOWN "shutdown"

#define CONFIG_NAMESPACE_COLOR "color"
#define CONFIG_COLOR_OVERLAY "overlay"
#define CONFIG_COLOR_TEXT "text"
#define CONFIG_COLOR_ERROR "error"
#define CONFIG_COLOR_SELECTED "selected"
#define CONFIG_COLOR_BUTTON "button"
#define CONFIG_COLOR_STATUS "status"
#define CONFIG_COLOR_WINDOW "window"

#ifndef SYSCONFDIR
#define SYSCONFDIR "/etc"
#endif

static config_t config = {0};

static bool file_exists(const char *path) {
  return path && access(path, R_OK) != -1;
}

// Most strings in this config can be distinguished by the first char
static inline bool streql(const char *a, const char *b) {
  return a[0] == b[0] && strcmp(a, b) == 0;
}

static void init(void) {
  config.font.icon = "FontAwesome";
  config.font.status = "monospace";
  config.font.text = "sans-serif";
  config.font.size = 16;

  config.actions[APP_OPTION_LOCK] = "loginctl lock-session";
  config.actions[APP_OPTION_SUSPEND] = "systemctl suspend-then-hibernate";
  config.actions[APP_OPTION_HIBERNATE] = "systemctl hibernate";
  config.actions[APP_OPTION_LOGOUT] = "loginctl terminate-session";
  config.actions[APP_OPTION_RESTART] = "systemctl reboot";
  config.actions[APP_OPTION_SHUTDOWN] = "systemctl poweroff";

  config.color.error = 0xff6b6bff;
  config.color.text = 0xeaeaeaff;
  config.color.overlay = 0x000000cc;
  config.color.selected = 0x82a2be80;
  config.color.button = 0x00000000;
  config.color.status = 0xc4c8c6ff;
  config.color.window = 0x00000000;
}

char *cfg_path(void) {
  static const char *config_paths[] = {
      "$XDG_CONFIG_HOME/" NAME "/config",
      SYSCONFDIR "/" NAME "/config",
  };

  char *config_home = getenv("XDG_CONFIG_HOME");
  if (!config_home || config_home[0] == '\0') {
    config_paths[1] = "$HOME/.config/" NAME "/config";
  }

  wordexp_t p;
  char *path;
  for (size_t i = 0; i < sizeof(config_paths) / sizeof(char *); ++i) {
    if (wordexp(config_paths[i], &p, 0) == 0) {
      path = strdup(p.we_wordv[0]);
      wordfree(&p);
      if (file_exists(path)) {
        return path;
      }
      free(path);
    }
  }

  return NULL;
}

void cfg_read(const char *path, config_t **cfg) {
#define readstr(Prop, Value)                                                   \
  if (streql(key, Value)) {                                                    \
    (Prop) = strdup(value);                                                    \
    continue;                                                                  \
  }
#define readcol(Prop, Value)                                                   \
  if (streql(key, Value)) {                                                    \
    (Prop) = color_from_string(value, Prop);                                   \
    continue;                                                                  \
  }

  init();
  *cfg = &config;

  if (!path) {
    log_info("no configuration path, using defaults", NULL);
    return;
  }

  FILE *config_file = fopen(path, "r");
  if (!config_file) {
    log_warn("failed to load config at '%s', using defaults", path);
    return;
  }

  char *line = NULL;
  size_t line_size = 0;
  ssize_t nread;
  int line_number = 0;
  log_debug("config", NULL);
  while ((nread = getline(&line, &line_size, config_file)) != -1) {
    line_number++;

    if (line[nread - 1] == '\n') {
      line[--nread] = '\0';
    }

    if (!*line || line[0] == CONFIG_COMMENT) {
      continue;
    }

    log_debug("  %d | %s", line_number, line);
    char *separator = strchr(line, CONFIG_VALUE_SEPARATOR);
    if (!separator) {
      log_warn("invalid line (missing %s), skipping", CONFIG_VALUE_SEPARATOR);
      continue;
    }

    *separator = '\0';
    char *value = separator + 1;

    char *dot = strchr(line, CONFIG_NAMESPACE_SEPARATOR);
    if (!dot) {
      log_warn("invalid line (missing %s), skipping",
               CONFIG_NAMESPACE_SEPARATOR);
      continue;
    }
    *dot = '\0';
    char *key = dot + 1;
    char *namespace = line;

    if (streql(namespace, CONFIG_NAMESPACE_FONT)) {
      readstr(config.font.text, CONFIG_FONT_TEXT);
      readstr(config.font.status, CONFIG_FONT_INFO);
      readstr(config.font.icon, CONFIG_FONT_ICON);

      if (streql(key, CONFIG_FONT_SIZE)) {
        // FIXME: this feels unsafe
        config.font.size = (uint32_t)atol(value);
        continue;
      }
    }

    if (streql(namespace, CONFIG_NAMESPACE_ACTIONS)) {
      readstr(config.actions[APP_OPTION_LOCK], CONFIG_ACTIONS_LOCK);
      readstr(config.actions[APP_OPTION_SUSPEND], CONFIG_ACTIONS_SUSPEND);
      readstr(config.actions[APP_OPTION_HIBERNATE], CONFIG_ACTIONS_HIBERNATE);
      readstr(config.actions[APP_OPTION_LOGOUT], CONFIG_ACTIONS_LOGOUT);
      readstr(config.actions[APP_OPTION_RESTART], CONFIG_ACTIONS_REBOOT);
      readstr(config.actions[APP_OPTION_SHUTDOWN], CONFIG_ACTIONS_SHUTDOWN);
    }

    if (streql(namespace, CONFIG_NAMESPACE_COLOR)) {
      readcol(config.color.overlay, CONFIG_COLOR_OVERLAY);
      readcol(config.color.text, CONFIG_COLOR_TEXT);
      readcol(config.color.error, CONFIG_COLOR_ERROR);
      readcol(config.color.selected, CONFIG_COLOR_SELECTED);
      readcol(config.color.button, CONFIG_COLOR_BUTTON);
      readcol(config.color.status, CONFIG_COLOR_STATUS);
      readcol(config.color.window, CONFIG_COLOR_WINDOW);
    }
  }

  free(line);
  fclose(config_file);
#undef readcol
#undef readstr
}
