#include <hb.h>
#include <stdio.h>

static void draw_glyph(hb_codepoint_t glyph, hb_position_t x, hb_position_t y) {
  if (!glyph) {
    fprintf(stderr, "<nil> @%d,%d\n", x, y);
    return;
  }

  unsigned int unicode = glyph - 100000;
  if (unicode >= 32 && unicode <= 126) {
    fprintf(stderr, "%c @%d,%d\n", unicode, x, y);
    return;
  }

  fprintf(stderr, "<?> @%d,%d\n", x, y);
}

static hb_bool_t nominal_glyph(struct hb_font_t * font, void * font_data, hb_codepoint_t unicode, hb_codepoint_t * glyph, void * user_data) {
  // Let's pretend we have a font that supports basic ASCII

  if (unicode >= 32 && unicode <= 126) {
    *glyph = unicode + 100000;
    return 1;
  }

  // 225/e1  = á
  // 769/301 = (combining acuteaccent)
  // 227/e3  = ã
  // 771/303 = (combining tilde)
  // 233/e9  = é
  // 161/a1  = ¡

  fprintf(stderr, "unsupported unicode: %d (%08x)\n", unicode, unicode);
  *glyph = 0;
  return 0;
}

static int get_h_advance(struct hb_font_t * font, void * font_data, hb_codepoint_t glyph, void * user_data) {
  unsigned int unicode = glyph - 100000;
  if (unicode >= 32 && unicode <= 126) return 5;
  return 0;
}

static hb_blob_t * ref_table(hb_face_t * face, hb_tag_t tag, void * user_data) {
  char * ctag = (char *)&tag;
  char tagp[5] = { ctag[3], ctag[2], ctag[1], ctag[0], 0 };
  fprintf(stderr, "ignoring request for reference table with tag '%s'\n", tagp);
  return NULL;
}

int main() {
  hb_buffer_t * buf = hb_buffer_create();
  hb_buffer_add_utf8(buf, "Olá mundão véi!¡", -1, 0, -1);
  hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
  hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
  hb_buffer_set_language(buf, hb_language_from_string("en", -1));

  hb_font_funcs_t * funcs = hb_font_funcs_create();
  hb_font_funcs_set_nominal_glyph_func(funcs, nominal_glyph, NULL, NULL);
  hb_font_funcs_set_glyph_h_advance_func(funcs, get_h_advance, NULL, NULL);

  hb_face_t * face = hb_face_create_for_tables(ref_table, NULL, NULL);
  hb_font_t * font = hb_font_create(face);
  hb_font_set_funcs(font, funcs, NULL, NULL);

  if (!hb_shape_full(font, buf, NULL, 0, NULL)) return 1;

  unsigned int glyph_count;
  hb_glyph_info_t * glyph_info    = hb_buffer_get_glyph_infos(buf, &glyph_count);
  hb_glyph_position_t * glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

  hb_position_t cursor_x = 0;
  hb_position_t cursor_y = 0;
  for (unsigned int i = 0; i < glyph_count; i++) {
    hb_codepoint_t glyphid   = glyph_info[i].codepoint;
    hb_position_t  x_offset  = glyph_pos[i].x_offset;
    hb_position_t  y_offset  = glyph_pos[i].y_offset;
    hb_position_t  x_advance = glyph_pos[i].x_advance;
    hb_position_t  y_advance = glyph_pos[i].y_advance;
    draw_glyph(glyphid, cursor_x + x_offset, cursor_y + y_offset);
    cursor_x += x_advance;
    cursor_y += y_advance;
  }

  hb_buffer_destroy(buf);
  hb_face_destroy(face);
  hb_font_destroy(font);
  hb_font_funcs_destroy(funcs);
}
