#pragma once

#include "ctx.h"
#include <cairo/cairo.h>
#include <stdint.h>

typedef uint32_t ui_color_t;

typedef struct {
  ui_color_t color;
  double size;
  const char *family;
  cairo_font_weight_t weight;
} ui_text_t;

typedef struct {
  double width, height;
  double y_bearing;
} ui_text_bounds_t;

void ui_init(ctx_t *c, ui_color_t background);

void ui_set_source_color(ui_color_t color);

void ui_text_init(ui_text_t opts, const char *text, ui_text_bounds_t *);
void ui_text_commit(double x, double y, const char *text);
void ui_text(double x, double y, ui_text_t opts, const char *text);

void ui_rect(double x, double y, double w, double h, ui_color_t color);

typedef enum {
  UI_BTN_STATUS_NONE,
  UI_BTN_STATUS_SELECTED,
  
  UI_BTN_STATUSES
} ui_btn_status_t;

typedef struct {
  ui_color_t bg;
  ui_color_t fg;
} ui_btn_colors_t;

typedef struct {
  const char *text_family;
  const char *icon_family;
  ui_btn_colors_t color[UI_BTN_STATUSES];
} ui_btn_t;

void ui_btn(double x, double y, double w, double h, ui_btn_t opts,
            const char *text, const char *icon, ui_btn_status_t status);
