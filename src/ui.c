#include "ui.h"
#include "assert.h"
#include "cairo-ft.h"
#include "cairo.h"
#include "log.h"
#include "../assets/font-awesome-v4.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdbool.h>
#include <stddef.h>

static cairo_t *cairo;

static cairo_font_face_t *load_default_icons(void) {
  static cairo_font_face_t *face = NULL;
  static bool attempted = false;

  if (attempted) {
    return face;
  }
  attempted = true;

  FT_Library library;
  if (FT_Init_FreeType(&library)) {
    log_error("failed to initialize freetype", NULL);
    return NULL;
  }

  FT_Face ft_face;
  if (FT_New_Memory_Face(library, (const FT_Byte *)fontawesome_bytes,
                         (FT_Long)fontawesome_bytes_len, 0, &ft_face)) {
    log_error("failed to load embedded icon font", NULL);
    return NULL;
  }

  face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);
  if (cairo_font_face_status(face) != CAIRO_STATUS_SUCCESS) {
    log_error("failed to create cairo font face for embedded icon font",
              NULL);
    face = NULL;
  }

  return face;
}

void ui_init(ctx_t *c, color_t background) {
  cairo = c->cairo.ctx;
  cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);
  cairo_identity_matrix(cairo);

  cairo_save(cairo);
  ui_set_source_color(background);
  cairo_set_operator(cairo, CAIRO_OPERATOR_SOURCE);
  cairo_paint(cairo);
  cairo_restore(cairo);
}

void ui_set_source_color(color_t color) {
  assert(cairo && "cairo must be non-null");

  cairo_set_source_rgba(cairo, (color >> (3 * 8) & 0xFF) / 255.0,
                        (color >> (2 * 8) & 0xFF) / 255.0,
                        (color >> (1 * 8) & 0xFF) / 255.0,
                        (color >> (0 * 8) & 0xFF) / 255.0);
}

void ui_txt_init(ui_txt_t opts, const char *text, ui_txt_bounds_t *bounds) {
  assert(bounds && "rect must be non-null");
  assert(cairo && "cairo must be non-null");

  cairo_font_options_t *font_options = cairo_font_options_create();

  cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_FULL);
  cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_SUBPIXEL);

  cairo_set_font_options(cairo, font_options);
  if (opts.family) {
    cairo_select_font_face(cairo, opts.family, CAIRO_FONT_SLANT_NORMAL,
                           opts.weight == UI_TXT_WEIGHT_BOLD
                               ? CAIRO_FONT_WEIGHT_BOLD
                               : CAIRO_FONT_WEIGHT_NORMAL);
  } else {
    cairo_font_face_t *icons = load_default_icons();
    if (icons) {
      cairo_set_font_face(cairo, icons);
    } else {
      cairo_select_font_face(cairo, "sans-serif", CAIRO_FONT_SLANT_NORMAL,
                             CAIRO_FONT_WEIGHT_NORMAL);
    }
  }
  cairo_set_font_size(cairo, opts.size);
  ui_set_source_color(opts.color);

  cairo_text_extents_t ext;
  cairo_text_extents(cairo, text, &ext);

  bounds->height = ext.height;
  bounds->width = ext.width;
  bounds->y_bearing = ext.y_bearing;

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

void ui_rect(double x, double y, double w, double h, color_t c) {
  ui_set_source_color(c);
  cairo_rectangle(cairo, x, y, w, h);
  cairo_fill(cairo);
}

void ui_btn(ui_btn_t opts, double x, double y, double w, double h,
            const char *text, const char *icon, ui_btn_status_t status) {
  double gap = opts.size;
  ui_rect(x, y, w, h, opts.color[status].bg);

  ui_txt_t icon_opts = {
      .color = opts.color[status].fg,
      .size = opts.size * 1.75,
      .family = opts.icon_family,
      .weight = UI_TXT_WEIGHT_NORMAL,
      .align = UI_TXT_ALIGN_CENTER,
  };
  ui_txt_t label_opts = icon_opts;
  label_opts.family = opts.text_family;
  label_opts.size = opts.size;

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
