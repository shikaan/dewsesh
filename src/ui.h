#pragma once

#include "ctx.h"
#include "color.h"
#include "result.h"
#include <stdint.h>

typedef enum {
  UI_TXT_ALIGN_LEFT,
  UI_TXT_ALIGN_CENTER,
  UI_TXT_ALIGN_RIGHT,
} ui_txt_align_t;

typedef enum {
  UI_TXT_WEIGHT_NORMAL,
  UI_TXT_WEIGHT_BOLD,

  UI_TXT_WEIGHTS
} ui_txt_weight_t;

typedef struct ui_font ui_font_t;

result_t ui_font_family(const char *family, ui_font_t **font);
result_t ui_font_embedded(ui_font_t **font);

typedef struct {
  color_t color;
  double size;
  const ui_font_t *font;
  ui_txt_weight_t weight;
  ui_txt_align_t align;
} ui_txt_t;

typedef struct {
  double width, height;
  double y_bearing;
} ui_txt_bounds_t;

result_t ui_init(void);

void ui_start_frame(ctx_t *c, color_t background);

void ui_set_source_color(color_t color);

void ui_txt_init(ui_txt_t opts, const char *text, ui_txt_bounds_t *bounds);
void ui_txt_commit(ui_txt_t opts, double x, double y,
                    const ui_txt_bounds_t *bounds, const char *text);
void ui_txt(ui_txt_t opts, double x, double y, const char *text);

void ui_rect(double x, double y, double w, double h, color_t color);

// Widgets

typedef enum {
  UI_BTN_STATUS_NONE,
  UI_BTN_STATUS_SELECTED,

  UI_BTN_STATUSES
} ui_btn_status_t;

typedef struct {
  color_t bg;
  color_t fg;
} ui_btn_colors_t;

typedef struct {
  const ui_font_t *text;
  const ui_font_t *icon;
  uint32_t size;
  ui_btn_colors_t color[UI_BTN_STATUSES];
} ui_btn_t;

void ui_btn(ui_btn_t opts, double x, double y, double w, double h,
            const char *text, const char *icon, ui_btn_status_t status);
