#include "config.h"
#include "color.h"

static bool file_exists(const char *path) {
  return path && access(path, R_OK) != -1;
}

char *get_config_path(void) {
  static const char *config_paths[] = {
      "$HOME/.dewlock/config",
      "$XDG_CONFIG_HOME/dewlock/config",
      SYSCONFDIR "/dewlock/config",
  };

  char *config_home = getenv("XDG_CONFIG_HOME");
  if (!config_home || config_home[0] == '\0') {
    config_paths[1] = "$HOME/.config/dewlock/config";
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

int load_config(char *path, struct dewlock_state *state) {
  FILE *config = fopen(path, "r");
  if (!config) {
    dewlock_log(LOG_ERROR, "Failed to read config. Running without it.");
    return 0;
  }
  char *line = NULL;
  size_t line_size = 0;
  ssize_t nread;
  int line_number = 0;
  while ((nread = getline(&line, &line_size, config)) != -1) {
    line_number++;

    if (line[nread - 1] == '\n') {
      line[--nread] = '\0';
    }

    if (!*line || line[0] == CONFIG_COMMENT) {
      continue;
    }

    dewlock_log(LOG_DEBUG, "Config Line #%d: %s", line_number, line);
    char *separator = strchr(line, CONFIG_VALUE_SEPARATOR);
    if (!separator) {
      dewlock_log(LOG_ERROR, "Invalid config line. Skipping.");
      continue;
    }

    *separator = '\0';
    char *value = separator + 1;

    char *dot = strchr(line, CONFIG_NAMESPACE_SEPARATOR);
    if (!dot) {
      dewlock_log(LOG_ERROR, "Invalid config line. Skipping.");
      continue;
    }
    *dot = '\0';
    char *key = dot + 1;
    char *namespace = line;

    if (namespace[0] == CONFIG_NAMESPACE_BACKGROUND[0] &&
        !strcmp(namespace, CONFIG_NAMESPACE_BACKGROUND)) {
      if (!strcmp(key, CONFIG_BACKGROUND_PATH)) {
        state->args.background.path = strdup(value);
        continue;
      }

      if (!strcmp(key, CONFIG_BACKGROUND_MODE)) {
        state->args.background.mode = parse_background_mode(value);
        continue;
      }
    }

    if (namespace[0] == CONFIG_NAMESPACE_FONT[0] &&
        !strcmp(namespace, CONFIG_NAMESPACE_FONT)) {
      if (!strcmp(key, CONFIG_FONT_FAMILY)) {
        state->args.font.family = strdup(value);
        continue;
      }

      if (!strcmp(key, CONFIG_FONT_SIZE)) {
        state->args.font.size = atoi(value);
        continue;
      }
    }

    if (namespace[0] == CONFIG_NAMESPACE_COLOR[0] &&
        !strcmp(namespace, CONFIG_NAMESPACE_COLOR)) {
      if (!strcmp(key, CONFIG_COLOR_BACKGROUND)) {
        state->args.colors.background = color_from_string(value);
        continue;
      }

      if (!strcmp(key, CONFIG_COLOR_OVERLAY)) {
        state->args.colors.overlay = parse_color(value);
        continue;
      }

      if (!strcmp(key, CONFIG_COLOR_TEXT)) {
        state->args.colors.text = parse_color(value);
        continue;
      }

      if (!strcmp(key, CONFIG_COLOR_WARNING)) {
        state->args.colors.warning = parse_color(value);
        continue;
      }

      if (!strcmp(key, CONFIG_COLOR_ERROR)) {
        state->args.colors.error = parse_color(value);
        continue;
      }
    }
  }
  free(line);
  fclose(config);
  return 0;
}

result_t config_read(config_t config**) {

  return OK;
}
