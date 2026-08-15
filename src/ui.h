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
  double w, h;
} ui_text_bounds_t;

void ui_init(ctx_t *c, ui_color_t background);

void ui_set_source_color(ui_color_t color);

void ui_text_init(ui_text_t opts, const char *text, ui_text_bounds_t *);
void ui_text_commit(double x, double y, const char *text);
void ui_text(double x, double y, ui_text_t opts, const char *text);

void ui_rect(double x , double y, double w, double h, ui_color_t color);
