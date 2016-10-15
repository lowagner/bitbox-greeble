#include "bitbox.h"
#include "common.h"
#include "font.h"
#include <string.h> // strlen
uint16_t font[256] CCM_MEMORY;
uint16_t font_cache[256] = {
  38505,
  150,
  26880,
  4808,
  4936,
  57890,
  39327,
  42405,
  65280,
  63281,
  65224,
  13448,
  34883,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  8226,
  85,
  64250,
  31326,
  55899,
  26102,
  34,
  25398,
  27846,
  2634,
  1252,
  17920,
  224,
  26112,
  4680,
  64447,
  63079,
  62407,
  63727,
  52477,
  31871,
  64831,
  26319,
  60375,
  64703,
  514,
  17922,
  1060,
  3598,
  578,
  5699,
  64399,
  57302,
  64471,
  59262,
  31671,
  62335,
  14143,
  64318,
  56829,
  63087,
  31951,
  47995,
  62259,
  40447,
  56763,
  27574,
  16319,
  64927,
  55223,
  64575,
  26223,
  64989,
  10171,
  65465,
  47947,
  26299,
  62415,
  29495,
  33825,
  60622,
  164,
  61440,
  66,
  64992,
  31603,
  58336,
  60908,
  59318,
  14142,
  32470,
  48947,
  58886,
  64524,
  47027,
  26214,
  40432,
  47984,
  27488,
  14263,
  52958,
  15216,
  30432,
  59126,
  64976,
  10160,
  64400,
  46256,
  32416,
  58992,
  25142,
  1092,
  25798,
  744,
  1632,
  12838,
  13119,
  64420,
  64959,
  56658,
  61551,
  56799,
  61807,
  46509,
  53849,
  48022,
  0,
  0,
  0,
  0,
  0,
  0,
  46440,
  64102,
  13673,
  28002,
  25062,
  34590,
  35223,
  28086,
  26208,
  47024,
  39491,
  36752,
  9904,
  57715,
  22000,
  13988,
  9696,
  36374,
  26352,
  27280,
  34401,
  50539,
  7786,
  27536,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  65535,
  8200,
  8216,
  12312,
  65280,
  4080,
  255,
  61455,
  61440,
  3840,
  240,
  15,
  4369,
  8738,
  17476,
  34952,
  0,
  61440,
  3840,
  65280,
  240,
  61680,
  4080,
  65520,
  15,
  61455,
  3855,
  65295,
  255,
  61695,
  4095,
  65535
};

void font_init()
{
    memcpy(font, font_cache, sizeof(font_cache));
}

void font_render_line_doubled(const uint8_t *text, int x, int y, uint16_t color_fg, uint16_t color_bg)
{
    #ifdef EMULATOR
    if (y < 0 || y >= 8)
    {
        message("got too big a line count for text (%s):  %d\n", text, y);
        return;
    }
    if (x < 0 || x + 9*strlen((char *)text) >= SCREEN_W)
    {
        message("text (%s) goes off screen!\n", text);
        return;
    }
    #endif
    y = ((y/2))*4; // make y now how much to shift
    uint16_t *dst = draw_buffer + x;
    uint16_t color_choice[2] = { color_bg, color_fg };
    *dst = color_choice[0];
    --text;
    int c;
    while ((c = *(++text)))
    {
        uint8_t row = (font[c] >> y) & 15;
        for (int j=0; j<4; ++j)
        {
            *(++dst) = color_choice[row&1];
            *(++dst) = color_choice[row&1];
            row >>= 1;
        }
        *(++dst) = color_choice[0];
    }
}

void font_render_no_bg_line_doubled(const uint8_t *text, int x, int y, uint16_t color_fg)
{
    #ifdef EMULATOR
    if (y < 0 || y >= 8)
    {
        message("got too big a line count for text (%s):  %d\n", text, y);
        return;
    }
    if (x < 0 || x + 9*strlen((char *)text) >= SCREEN_W)
    {
        message("text (%s) goes off screen!\n", text);
        return;
    }
    #endif
    y = ((y/2))*4; // make y now how much to shift
    uint16_t *dst = draw_buffer + x;
    --text;
    int c;
    while ((c = *(++text)))
    {
        uint8_t row = (font[c] >> y) & 15;
        for (int j=0; j<4; ++j)
        {
            if (row&1)
            {
                *(++dst) = color_fg;
                *(++dst) = color_fg;
            }
            else
            {
                dst += 2;
            }
            row >>= 1;
        }
        ++dst;
    }
}
