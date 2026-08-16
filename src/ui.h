#pragma once

#include "ctx.h"
#include <cairo/cairo.h>
#include <stdint.h>

typedef uint32_t ui_color_t;

typedef enum {
  UI_TXT_ALIGN_LEFT,
  UI_TXT_ALIGN_CENTER,
  UI_TXT_ALIGN_RIGHT,
} ui_txt_align_t;

typedef struct {
  ui_color_t color;
  double size;
  const char *family;
  cairo_font_weight_t weight;
  ui_txt_align_t align;
} ui_txt_t;

typedef struct {
  double width, height;
  double y_bearing;
} ui_txt_bounds_t;

void ui_init(ctx_t *c, ui_color_t background);

void ui_set_source_color(ui_color_t color);

void ui_text_init(ui_txt_t opts, const char *text, ui_txt_bounds_t *bounds);
void ui_text_commit(ui_txt_t opts, double x, double y,
                    const ui_txt_bounds_t *bounds, const char *text);
void ui_text(ui_txt_t opts, double x, double y, const char *text);

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

void ui_btn(ui_btn_t opts, double x, double y, double w, double h,
            const char *text, const char *icon, ui_btn_status_t status);
