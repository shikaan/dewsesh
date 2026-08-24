#include "ui.h"
#include "assert.h"
#include "cairo-ft.h"
#include "cairo.h"
#include "log.h"
#include "result.h"
#include "../assets/font-awesome-v4.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdbool.h>
#include <stddef.h>

enum { FONTS = 3 };

struct ui_font {
  cairo_font_face_t *face[UI_TXT_WEIGHTS];
};

static cairo_t *cairo;
static cairo_font_options_t *font_options = NULL;
static FT_Library ft_library = NULL;
static struct ui_font fonts[FONTS] = {0};
static size_t nfonts = 0;

static result_t font_slot(ui_font_t **font) {
  assert(font && "font must be non-null");

  if (nfonts == FONTS) {
    log_error("cannot register font. Fonts list is full %d", FONTS);
    return ERR_UI_TOO_MANY_FONTS;
  }

  *font = &fonts[nfonts];
  return OK;
}

result_t ui_font_family(const char *family, ui_font_t **font) {
  assert(family && "family must be non-null");

  static const cairo_font_weight_t weights[UI_TXT_WEIGHTS] = {
      [UI_TXT_WEIGHT_NORMAL] = CAIRO_FONT_WEIGHT_NORMAL,
      [UI_TXT_WEIGHT_BOLD] = CAIRO_FONT_WEIGHT_BOLD,
  };

  result_t result = font_slot(font);
  if (result != OK) {
    return result;
  }

  for (size_t i = 0; i < UI_TXT_WEIGHTS; i++) {
    cairo_font_face_t *face =
        cairo_toy_font_face_create(family, CAIRO_FONT_SLANT_NORMAL, weights[i]);

    if (cairo_font_face_status(face) != CAIRO_STATUS_SUCCESS) {
      log_error("failed to create font face for family '%s'", family);
      return ERR_UI_FONT;
    }

    (*font)->face[i] = face;
  }

  nfonts++;
  return OK;
}

result_t ui_font_embedded(ui_font_t **font) {
  assert(ft_library && "ui_init must be called before loading fonts");

  FT_Face ft_face;
  if (FT_New_Memory_Face(ft_library, (const FT_Byte *)fontawesome_bytes,
                         (FT_Long)fontawesome_bytes_len, 0, &ft_face)) {
    log_error("failed to load embedded icon font", NULL);
    return ERR_UI_FONT;
  }

  cairo_font_face_t *face = cairo_ft_font_face_create_for_ft_face(ft_face, 0);
  if (cairo_font_face_status(face) != CAIRO_STATUS_SUCCESS) {
    log_error("failed to create cairo font face for embedded icon font", NULL);
    FT_Done_Face(ft_face);
    return ERR_UI_FONT;
  }

  result_t result = font_slot(font);
  if (result != OK) {
    cairo_font_face_destroy(face);
    FT_Done_Face(ft_face);
    return result;
  }

  for (size_t i = 0; i < UI_TXT_WEIGHTS; i++) {
    (*font)->face[i] = face;
  }

  nfonts++;
  return OK;
}

result_t ui_init(void) {
  if (FT_Init_FreeType(&ft_library)) {
    log_error("failed to initialize freetype", NULL);
    return ERR_UI;
  }

  font_options = cairo_font_options_create();
  if (cairo_font_options_status(font_options) != CAIRO_STATUS_SUCCESS) {
    log_error("failed to create font options", NULL);
    return ERR_UI;
  }

  cairo_font_options_set_hint_style(font_options, CAIRO_HINT_STYLE_FULL);
  cairo_font_options_set_antialias(font_options, CAIRO_ANTIALIAS_SUBPIXEL);

  return OK;
}

void ui_start_frame(ctx_t *c, color_t background) {
  assert(font_options && "ui_init must be called before drawing");

  cairo = c->cairo.ctx;
  cairo_set_antialias(cairo, CAIRO_ANTIALIAS_BEST);
  cairo_set_font_options(cairo, font_options);
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
  assert(opts.font && "font must be non-null");

  cairo_set_font_face(cairo, opts.font->face[opts.weight]);
  cairo_set_font_size(cairo, opts.size);
  ui_set_source_color(opts.color);

  cairo_text_extents_t ext;
  cairo_text_extents(cairo, text, &ext);

  bounds->height = ext.height;
  bounds->width = ext.width;
  bounds->y_bearing = ext.y_bearing;
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
      .font = opts.icon,
      .weight = UI_TXT_WEIGHT_NORMAL,
      .align = UI_TXT_ALIGN_CENTER,
  };
  ui_txt_t label_opts = icon_opts;
  label_opts.font = opts.text;
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
