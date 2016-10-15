#ifndef FONT_H
#define FONT_H
#include <stdint.h>
extern uint16_t font_cache[256];
extern uint16_t font[256];
void font_init();
void font_render_line_doubled(const uint8_t *text, int x, int y, uint16_t color_fg, uint16_t color_bg);
void font_render_no_bg_line_doubled(const uint8_t *text, int x, int y, uint16_t color_fg);
#endif
