#include "ui.h"
#include "assert.h"
#include "cairo.h"

static cairo_t *cairo;

void ui_init(ctx_t *c, ui_color_t background) { 
  cairo = c->cairo.ctx;
  cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);
  cairo_identity_matrix(cairo);

  cairo_save(cairo);
  ui_set_source_color(background);
  cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cairo);
  cairo_restore(cairo);
}

void ui_set_source_color(ui_color_t color) {
  assert(cairo && "cairo must be non-null");

  cairo_set_source_rgba(cairo, (color >> (3 * 8) & 0xFF) / 255.0,
                        (color >> (2 * 8) & 0xFF) / 255.0,
                        (color >> (1 * 8) & 0xFF) / 255.0,
                        (color >> (0 * 8) & 0xFF) / 255.0);
}

void ui_text_init(ui_text_t opts, const char *text, ui_text_bounds_t *extents) {
  assert(extents && "rect must be non-null");
  assert(cairo && "cairo must be non-null");

  cairo_font_options_t *font_options = cairo_font_options_create();

  cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_FULL);
  cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_SUBPIXEL);

  cairo_set_font_options(cairo, font_options);
  cairo_select_font_face(cairo, opts.family, CAIRO_FONT_SLANT_NORMAL,
                         opts.weight);
  cairo_set_font_size(cairo, opts.size);
  ui_set_source_color(opts.color);

  cairo_text_extents_t ext;
  cairo_text_extents(cairo, text, &ext);

  extents->h = ext.height;
  extents->w = ext.width;

  cairo_font_options_destroy(font_options);
}

void ui_text_commit(double x, double y, const char *text) {
  assert(cairo && "cairo must be non-null");
  assert(text && "text must be non-null");

  cairo_move_to(cairo, x, y);
  cairo_show_text(cairo, text);
}

void ui_text(double x, double y, ui_text_t opts, const char *text) {
  assert(cairo && "cairo must be non-null");
  assert(text && "text must be non-null");

  ui_text_bounds_t r;
  ui_text_init(opts, text, &r);
  ui_text_commit(x, y, text);
}

void ui_rect(double x , double y, double w, double h, ui_color_t c) {
  ui_set_source_color(c);
  cairo_rectangle(cairo, x, y, w, h);
  cairo_fill(cairo);
}
