#include "ui.h"
#include "assert.h"
#include "cairo.h"
#include <stddef.h>

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

void ui_txt_init(ui_txt_t opts, const char *text, ui_txt_bounds_t *extents) {
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

  extents->height = ext.height;
  extents->width = ext.width;
  extents->y_bearing = ext.y_bearing;

  cairo_font_options_destroy(font_options);
}

void ui_txt_commit(ui_txt_t opts, double x, double y,
                   const ui_txt_bounds_t *bounds, const char *text) {
  assert(cairo && "cairo must be non-null");
  assert(text && "text must be non-null");
  assert(bounds && "bounds must be non-null");

  double relx = x;
  switch (opts.align) {
  case UI_TXT_ALIGN_LEFT:
    relx = x;
    break;
  case UI_TXT_ALIGN_CENTER:
    relx = x - bounds->width / 2;
    break;
  case UI_TXT_ALIGN_RIGHT:
    relx = x - bounds->width;
    break;
  default:
    assert(false && "unreacheable");
  }

  cairo_move_to(cairo, relx, y);
  cairo_show_text(cairo, text);
}

void ui_txt(ui_txt_t opts, double x, double y, const char *text) {
  assert(cairo && "cairo must be non-null");
  assert(text && "text must be non-null");

  ui_txt_bounds_t r;
  ui_txt_init(opts, text, &r);
  ui_txt_commit(opts, x, y, &r, text);
}

void ui_rect(double x, double y, double w, double h, ui_color_t c) {
  ui_set_source_color(c);
  cairo_rectangle(cairo, x, y, w, h);
  cairo_fill(cairo);
}

void ui_btn(ui_btn_t opts, double x, double y, double w, double h,
            const char *text, const char *icon, ui_btn_status_t status) {
  double gap = 16;
  ui_rect(x, y, w, h, opts.color[status].bg);

  ui_txt_t icon_opts = {
      .color = opts.color[status].fg,
      .size = 28,
      .family = opts.icon_family,
      .weight = CAIRO_FONT_WEIGHT_NORMAL,
      .align = UI_TXT_ALIGN_CENTER,
  };
  ui_txt_t label_opts = icon_opts;
  label_opts.family = opts.text_family;
  label_opts.size = 16;

  ui_txt_bounds_t icon_bounds, label_bounds;
  ui_txt_init(icon_opts, icon, &icon_bounds);
  ui_txt_init(label_opts, text, &label_bounds);

  double content_height = icon_bounds.height + gap + label_bounds.height;
  double top = y + (h - content_height) / 2;
  double icony = top - icon_bounds.y_bearing;
  double labely = top + icon_bounds.height + gap - label_bounds.y_bearing;

  ui_txt_commit(label_opts, x + w / 2, labely, &label_bounds, text);

  ui_txt_init(icon_opts, icon, &icon_bounds);
  ui_txt_commit(icon_opts, x + w / 2, icony, &icon_bounds, icon);
}
