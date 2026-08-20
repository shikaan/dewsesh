#pragma once

#include "ctx.h"
#include "color.h"
#include <stdint.h>

typedef enum {
  UI_TXT_ALIGN_LEFT,
  UI_TXT_ALIGN_CENTER,
  UI_TXT_ALIGN_RIGHT,
} ui_txt_align_t;

typedef enum {
  UI_TXT_WEIGHT_NORMAL,
  UI_TXT_WEIGHT_BOLD,
} ui_txt_weight_t;

typedef struct {
  color_t color;
  double size;
  const char *family;
  ui_txt_weight_t weight;
  ui_txt_align_t align;
} ui_txt_t;

typedef struct {
  double width, height;
  double y_bearing;
} ui_txt_bounds_t;

void ui_init(ctx_t *c, color_t background);

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
  const char *text_family;
  const char *icon_family;
  uint32_t size;
  ui_btn_colors_t color[UI_BTN_STATUSES];
} ui_btn_t;

void ui_btn(ui_btn_t opts, double x, double y, double w, double h,
            const char *text, const char *icon, ui_btn_status_t status);
