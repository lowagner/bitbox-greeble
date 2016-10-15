#include "palette.h"
#include "common.h"
#include "bitbox.h"
#include "font.h"
#include "name.h"
#include "io.h"

#include "string.h" //memcpy

uint16_t palette[16] CCM_MEMORY;

void palette_reset()
{
    uint16_t colors[256];
    int k=-1;
    int grays[8] = { 0, 4, 8, 12, 17, 22, 27, 31 };
    for (int gray=0; gray<8; ++gray)
    {
        int gsg = grays[gray];
        colors[2*(++k)] = (gsg<<10)|(gsg<<5)|(gsg);
        gsg = (grays[gray] + 14)/2;
        colors[2*k + 1] = (gsg<<10)|(gsg<<5)|(gsg);
    }
    int rs[5] = { 0, 8<<10, 16<<10, 24<<10, 31<<10 };
    int gs[6] = { 0, 6<<5, 12<<5, 19<<5, 25<<5, 31<<5 };
    int bs[4] = { 0, 10, 21, 31 };
    for (int r=0; r<5; ++r)
    for (int g=0; g<6; ++g)
    for (int b=0; b<4; ++b)
    {
        uint16_t c = rs[r] | gs[g] | bs[b];
        colors[2*(++k)] = c;
        colors[2*k + 1] = ~c;
    }
    memcpy(palette_block_data, colors, sizeof(colors));
}

#define NUMBER_LINES 12

uint8_t palette_index CCM_MEMORY;
uint8_t palette_cursor CCM_MEMORY;
uint32_t palette_copying CCM_MEMORY;

void palette_init()
{
    palette_index = 0;
    palette_cursor = 0; // runs from 0 to 6, for fg (x3) to bg (x3)
    palette_copying = (1<<31);

    static const uint16_t colors[16] = {
        RGB(0, 0, 0),
        RGB(157, 157, 157),
        (1<<15) - 1,
        RGB(224, 111, 139),
        RGB(190, 38, 51),
        RGB(235, 137, 49),
        RGB(164, 100, 34),
        RGB(73, 60, 43),
        RGB(247, 226, 107),
        RGB(163, 206, 39),
        RGB(68, 137, 26),
        RGB(47, 72, 78),
        RGB(178, 220, 239),
        RGB(49, 162, 242),
        RGB(0, 87, 132),
        RGB(28, 20, 40)
    };
    memcpy(palette, colors, sizeof(colors));
}

void palette_line()
{
    if (vga_line < 22)
    {
        if (vga_line/2 == 0)
            memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= 22 + NUMBER_LINES*10)
    {
        if (vga_line == (22+NUMBER_LINES*10))
            memset(draw_buffer, 0, 2*SCREEN_W);
        return;
    }
    int line = (vga_line-22) / 10;
    int internal_line = (vga_line-22) % 10;
    if (internal_line == 0 || internal_line == 9)
    {
        memset(draw_buffer, 0, 2*SCREEN_W);
        if (line == 8)
        {
            // RGB line, add a little underline to currently selected color
            memset(draw_buffer + 30 + (palette_cursor + palette_cursor/3)*4*9, 255, 2*3*9);
        }
    }
    else
    {
        --internal_line;
        switch (line)
        {
        case 0:
            font_render_line_doubled((uint8_t *)"palette in", 16, internal_line, 65535, 0);
            font_render_line_doubled((uint8_t *)base_filename, 16+9*11, internal_line, 65535, 0);
            break;
        case 2:
            font_render_line_doubled((const uint8_t *)"L/R:cycle index", 16, internal_line, 65535, 0);
            {
            uint8_t label[] = { hex[palette_index/8], hex[palette_index%8], ':', 0 };
            font_render_line_doubled(label, 16 + 9*21, internal_line, 65535, 0);
            }
            break;
        case 3:
            if (palette_copying < 1<<31)
                font_render_line_doubled((const uint8_t *)"A:cancel copy", 16+2*9, internal_line, 65535, 0);
            else
                font_render_line_doubled((const uint8_t *)"A:save color", 16+2*9, internal_line, 65535, 0);
            break;
        case 4:
            if (palette_copying < 1<<31)
                font_render_line_doubled((const uint8_t *)"X:  \"     \"", 16+2*9, internal_line, 65535, 0);
            else
                font_render_line_doubled((const uint8_t *)"X:copy color", 16+2*9, internal_line, 65535, 0);
            break;
        case 5:
            if (palette_copying < 1<<31)
                font_render_line_doubled((const uint8_t *)"B:  \"     \"", 16+2*9, internal_line, 65535, 0);
            else
                font_render_line_doubled((const uint8_t *)"B:load color", 16+2*9, internal_line, 65535, 0);
            break;
        case 6:
            if (palette_copying < 1<<31)
                font_render_line_doubled((const uint8_t *)"Y:paste", 16+2*9, internal_line, 65535, 0);
            else
                font_render_line_doubled((const uint8_t *)"Y:change filename", 16+2*9, internal_line, 65535, 0);
            break;
        case 7:
            break;
        case 8:
        {
            uint32_t color = palette_block_data[palette_index];
            // foreground
            uint8_t label[] = { 'r', ':', hex[(color>>10)&31], 0 };
            font_render_line_doubled(label, 30, internal_line, RGB(255, 50, 50), 0);
            label[0] = 'g'; label[2] = hex[(color>>5)&31];
            font_render_line_doubled(label, 30+4*9, internal_line, RGB(50, 255, 50), 0);
            label[0] = 'b'; label[2] = hex[(color)&31];
            font_render_line_doubled(label, 30+8*9, internal_line, RGB(50, 100, 255), 0);
            // background
            label[0] = 'r'; label[2] = hex[(color>>26)&31];
            font_render_line_doubled(label, 30+16*9, internal_line, RGB(255, 50, 50), 0);
            label[0] = 'g'; label[2] = hex[(color>>21)&31];
            font_render_line_doubled(label, 30+20*9, internal_line, RGB(50, 255, 50), 0);
            label[0] = 'b'; label[2] = hex[(color>>16)&31];
            font_render_line_doubled(label, 30+24*9, internal_line, RGB(50, 100, 255), 0);
            break;
        }
        case 10:
            font_render_line_doubled((const uint8_t *)"select:edit music", 16, internal_line, 65535, 0);
            break;
        case 11:
            font_render_line_doubled(game_message, 16, internal_line, 65535, 0);
            break;
        }
    }
    if (vga_line < 22 + 2*16)
    {
        uint32_t *dst = (uint32_t *)draw_buffer + (SCREEN_W - 24 - 16*2*2)/2 - 1;
        uint32_t color = palette_block_data[palette_index];
        switch (vga_line/2)
        {
            case 22/2:
            case (21+2*16)/2:
                color = (color>>16) | (color & (65535<<16));
                for (int l=0; l<16; ++l) 
                    *(++dst) = color;
                break;
            default:
            {
                uint32_t final_color = (color>>16) | (color&(65535<<16)); // bg
                *(++dst) = final_color;
                color = (color&65535) | (color<<16);
                for (int l=1; l<15; ++l) 
                    *(++dst) = color;
                *(++dst) = final_color;
            }
        }
        ++dst;
        for (int l=0; l<8; ++l)
        {
            color = palette_block_data[(l + ((vga_line - 22))/2 * 8)&127];
            color = (color&65535) | (color<<16); // just keep fg color

            *(++dst) = color;
            *(++dst) = color;
        }
    }
    else if (vga_line/2 == (22 + 2*16)/2 || vga_line/2 == (22 + 4*16 + 2)/2)
    {
        memset(draw_buffer+(SCREEN_W - 24 - 16*2*2), 0, 64+4+64);
    }
    else if (vga_line < 22 + 2 + 2*16 + 2*16)
    {
        uint32_t *dst = (uint32_t *)draw_buffer + (SCREEN_W - 24 - 16*4)/2 - 1;
        uint32_t color; 
        for (int l=0; l<8; ++l)
        {
            color = palette_block_data[(l + ((vga_line - (22+32+2)))/2 * 8)&127];
            color = (color&65535) | (color<<16); // just keep fg color

            *(++dst) = color;
            *(++dst) = color;
        }
        if (palette_copying < (1<<31))
        {
            ++dst;
            color = palette_copying;
            switch (vga_line/2)
            {
                case (22+2+2*16)/2:
                case (21+2+2*16+2*16)/2:
                    color = (color>>16) | (color & (65535<<16));
                    for (int l=0; l<16; ++l) 
                        *(++dst) = color;
                    break;
                default:
                {
                    uint32_t final_color = (color>>16) | (color&(65535<<16)); // bg
                    *(++dst) = final_color;
                    color = (color&65535) | (color<<16);
                    for (int l=1; l<15; ++l) 
                        *(++dst) = color;
                    *(++dst) = final_color;
                }
            }
        }
    }
}

void palette_controls()
{
    int moved = 0;
    if (GAMEPAD_PRESSING(0, R))
    {
        ++moved;
    }
    if (GAMEPAD_PRESSING(0, L))
    {
        --moved;
    }
    if (moved)
    {
        game_message[0] = 0;
        palette_index = (palette_index + moved)&127;
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;
        return;
    }
    if (GAMEPAD_PRESS(0, right))
    {
        if (palette_cursor < 5)
            ++palette_cursor;
        else
            palette_cursor = 0;
        return;
    }
    if (GAMEPAD_PRESS(0, left))
    {
        if (palette_cursor)
            --palette_cursor;
        else
            palette_cursor = 5;
        return;
    }
    int make_wait = 0;
    if (GAMEPAD_PRESSING(0, up))
    {
        game_message[0] = 0;
        if (palette_cursor < 3)
        {
            if (((palette_block_data[palette_index] >> (10-5*palette_cursor)) & 31) < 31)
                palette_block_data[palette_index] += 1 << (10-5*palette_cursor);
        }
        else
        {
            if (((palette_block_data[palette_index] >> (16+25-5*palette_cursor)) & 31) < 31)
                palette_block_data[palette_index] += 1 << (16+25-5*palette_cursor);
        }
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, down))
    {
        game_message[0] = 0;
        if (palette_cursor < 3)
        {
            if (((palette_block_data[palette_index] >> (10-5*palette_cursor)) & 31) > 0)
                palette_block_data[palette_index] -= 1 << (10-5*palette_cursor);
        }
        else
        {
            if (((palette_block_data[palette_index] >> (16+25-5*palette_cursor)) & 31) > 0)
                palette_block_data[palette_index] -= 1 << (16+25-5*palette_cursor);
        }
        make_wait = 1;
    }
    if (make_wait)
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;

    if (GAMEPAD_PRESS(0, X))
    {
        // copy or uncopy
        if (palette_copying < 1<<31)
            palette_copying = 1<<31;
        else
            palette_copying = palette_block_data[palette_index];
        return;
    }
    int save_or_load = 0;
    if (GAMEPAD_PRESS(0, A))
    {
        // save
        save_or_load = 1;
    }
    else if (GAMEPAD_PRESS(0, B))
    {
        // load
        save_or_load = 2;
    }
    if (save_or_load)
    {
        if (palette_copying < 1<<31)
        {
            // or cancel a copy
            palette_copying = 1<<31;
            return;
        }
            
        FileError error;
        if (save_or_load == 1) // save
            error = io_save_palette(palette_index);
        else // load
            error = io_load_palette(palette_index);
        io_message_from_error(game_message, error, save_or_load);
        return;
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        if (palette_copying < 1<<31)
        {
            // paste:
            strcpy((char *)game_message, "pasted.");
            palette_block_data[palette_index] = palette_copying;
            palette_copying = 1<<31;
        }
        else
        {
            // go to filename chooser
            game_message[0] = 0;
            previous_visual_mode = EditPalette;
            game_switch(ChooseFilename);
        }
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        game_switch(EditAnthem);
        previous_visual_mode = None;
        return;
    }
    if (GAMEPAD_PRESS(0, start))
    {
        game_message[0] = 0;
        if (previous_visual_mode)
        {
            game_switch(previous_visual_mode);
            previous_visual_mode = None;
        }
        else
        {
            game_switch(SaveLoadScreen);
        }
        return;
    }
}
