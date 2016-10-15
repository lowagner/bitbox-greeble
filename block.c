#include "bitbox.h"
#include "common.h"
#include "font.h"
#include "name.h"
#include "io.h"
#include "view.h"

#include <string.h> // memset

#define MENU_COLOR 136 // a uint8_t, uint16_t color is (_COLOR)|(_COLOR<<8)
#define BLOCK_COLOR 164
#define BG_COLOR (block_menu ? MENU_COLOR : BLOCK_COLOR)

#define NUMBER_LINES 19

uint8_t block_copying CCM_MEMORY; // 0 for not copying, 1 for sprite, 2 for tile
uint8_t block_index CCM_MEMORY;
uint8_t block_cursor CCM_MEMORY;
uint8_t block_menu CCM_MEMORY;

void block_init()
{
    block_index = 1;
    block_copying = 128;
    block_menu = 0;
    block_cursor = 2;
}

void block_reset()
{
    for (int i=0; i<128; ++i)
        palette_block_data[128+i] = i * 16843009;
}

void block_line()
{
    if (vga_line < 22)
    {
        if (vga_line/2 == 0)
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= 22 + NUMBER_LINES*10)
    {
        if (vga_line/2 == (22+NUMBER_LINES*10)/2)
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }
    int line = (vga_line-22) / 10;
    int internal_line = (vga_line-22) % 10;
    if (internal_line == 0 || internal_line == 9)
    {
        memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        if (block_cursor < 6)
        {
            if (line == 13)
                memset(&draw_buffer[32+9+9*block_cursor+36*(block_cursor/3)], 229, 9*2);
        }
        else if (line == 15)
        {
            memset(&draw_buffer[32+9+9*(block_cursor-6)+36*((block_cursor-6)/3)], 229, 9*2);
        }
    }
    else
    {
        --internal_line;
        switch (line)
        {
        case 0:
        {
            uint8_t label[] = { 
                'b', 'l', 'o', 'c', 'k', ' ', hex[block_index/8], hex[block_index%8], 
            0 };
            font_render_line_doubled(label, 16, internal_line, 65535, BG_COLOR*257);
            break;
        }
        case 1: 
            break;
        case 2:
            font_render_line_doubled((const uint8_t *)"L/R:cycle block", 16, internal_line, 65535, BG_COLOR*257);
            break;
        case 3:
            if (block_copying < 128)
                font_render_line_doubled((const uint8_t *)"A:cancel copy", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            else
                font_render_line_doubled((const uint8_t *)"A:save to file", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            break;
        case 4:
            if (block_copying < 128)
                font_render_line_doubled((const uint8_t *)"B:  \"     \"", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            else
                font_render_line_doubled((const uint8_t *)"B:load from file", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            break;
        case 5:
            if (block_copying < 128)
                font_render_line_doubled((const uint8_t *)"X:  \"     \"", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            else
                font_render_line_doubled((const uint8_t *)"X:copy", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            break;
        case 6:
            if (block_copying < 128)
                font_render_line_doubled((const uint8_t *)"Y:paste", 16+2*9, internal_line, 65535, 257*BG_COLOR);
            else
                font_render_line_doubled((const uint8_t *)"Y:change filename", 16+2*9, internal_line, 65535, 257*BG_COLOR);
        case 7:
            break;
        case 8:
            if (previous_visual_mode)
                font_render_line_doubled((const uint8_t *)"start:back", 16, internal_line, 65535, 257*BG_COLOR);
            else
                font_render_line_doubled((const uint8_t *)"start:view block", 16, internal_line, 65535, 257*BG_COLOR);
            break;
        case 9:
            font_render_line_doubled((const uint8_t *)"select:edit palette", 16, internal_line, 65535, BG_COLOR*257);
            break;
        case 11:
            font_render_line_doubled((const uint8_t *)"dpad:adjust block", 16, internal_line, 65535, BG_COLOR*257);
            break;
        case 13:
        {
            uint32_t data = palette_block_data[128+block_index];
            uint8_t sub_data = (data>>8)&255;
            uint8_t msg[] = {
                (sub_data & 128) ? 'B' : 'C', 
                hex[(sub_data&127)/8], '0'+(sub_data&127)%8,
            0 };
            font_render_line_doubled(msg, 32+9, internal_line, 65535, BG_COLOR*257);
            msg[0] = (data & 128) ? 'B' : 'C';
            msg[1] = hex[(data&127)/8];
            msg[2] = '0'+(data&127)%8;
            font_render_line_doubled(msg, 32+9*8, internal_line, 65535, BG_COLOR*257);
            break;
        }
        case 15:
        {
            uint32_t data = palette_block_data[128+block_index];
            uint8_t sub_data = (data>>16)&255;
            uint8_t msg[] = {
                (sub_data & 128) ? 'B' : 'C', 
                hex[(sub_data&127)/8], '0'+(sub_data&127)%8,
            0 };
            font_render_line_doubled(msg, 32+9, internal_line, 65535, BG_COLOR*257);
            sub_data = (data>>24)&255;
            msg[0] = (sub_data & 128) ? 'B' : 'C';
            msg[1] = hex[(sub_data&127)/8];
            msg[2] = '0'+(sub_data&127)%8;
            font_render_line_doubled(msg, 32+9*8, internal_line, 65535, BG_COLOR*257);
            break;
        }
        case (NUMBER_LINES-2):
            if (game_message[0])
                font_render_line_doubled(game_message, 16, internal_line, 65535, BG_COLOR*257);
            else
                font_render_line_doubled((const uint8_t *)"B/C:block/color", 24, internal_line, 65535, BG_COLOR*257);
            break;
        }
    }
    if (vga_line < 22 + 2 + 2*16 + 2*16 && (block_copying<128))
    {
        int ix = SCREEN_W-24 - 16*2 + 2;
        int final_ix = ix + 32; // final image coordinate, not inclusive
        int x = 0;
        int y = vga_line - (22 + 2 + 2*16 + 2*16);
        while (ix < final_ix)
        {
            uint32_t color, length;
            drill_down(&color, &length, palette_block_data[128+block_index], x, y, 32);
            x += length;
            int next_ix = ix + length;
            do
                draw_buffer[ix++] = color & 65535; // need to add bg sometime
            while (ix < next_ix);
        }
    }
    else if (vga_line >= 22 + NUMBER_LINES*10 - 64)
    {
        int ix = SCREEN_W-30 - 16*2*2;
        int final_ix = ix + 64; // final image coordinate, not inclusive
        int x = 0;
        int y = vga_line - (22 + NUMBER_LINES*10 - 64);
        while (ix < final_ix)
        {
            uint32_t color, length;
            drill_down(&color, &length, palette_block_data[128+block_index], x, y, 64);
            x += length;
            int next_ix = ix + length;
            do
                draw_buffer[ix++] = color & 65535; // need to add bg sometime
            while (ix < next_ix);
        }
    }
}

void block_controls()
{
    if (GAMEPAD_PRESSING(0, R))
    {
        if (++block_index == 128)
            block_index = 0;
        game_message[0] = 0;
        gamepad_press_wait = GAMEPAD_PRESS_WAIT+GAMEPAD_PRESS_WAIT/2;
        return;
    }
    if (GAMEPAD_PRESSING(0, L))
    {
        if (block_index)
            --block_index;
        else
            block_index = 127;
        game_message[0] = 0;
        gamepad_press_wait = GAMEPAD_PRESS_WAIT+GAMEPAD_PRESS_WAIT/2;
        return;
    }
    if (GAMEPAD_PRESS(0, left))
    {
        if (block_cursor)
            --block_cursor;
        else
            block_cursor = 11;
        return;
    }
    if (GAMEPAD_PRESS(0, right))
    {
        if (++block_cursor > 11)
            block_cursor = 0;
        return;
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        const int swapping[4] = { 8, 0, 16, 24 };
        uint32_t data = palette_block_data[128+block_index];
        uint8_t sub_data = (data >> swapping[block_cursor/3]) & 255;
        switch (block_cursor % 3)
        {
            case 0:
                if (sub_data & 128)
                    sub_data &= ~128;
                else
                    sub_data |= 128;
                break;
            case 1:
                if ((sub_data/8)%16 == 15)
                    sub_data -= 15*8;
                else
                    sub_data += 8;
                break;
            case 2:
                if (sub_data % 8 == 7)
                    sub_data -= 7;
                else
                    ++sub_data;
                break;
        }
        palette_block_data[128+block_index] &= ~(255<<swapping[block_cursor/3]);
        palette_block_data[128+block_index] |= sub_data<<swapping[block_cursor/3];
        gamepad_press_wait = GAMEPAD_PRESS_WAIT+GAMEPAD_PRESS_WAIT/2;
        return;
    }
    if (GAMEPAD_PRESSING(0, down))
    {
        const int swapping[4] = { 8, 0, 16, 24 };
        uint32_t data = palette_block_data[128+block_index];
        uint8_t sub_data = (data >> swapping[block_cursor/3]) & 255;
        switch (block_cursor % 3)
        {
            case 0:
                if (sub_data & 128)
                    sub_data &= ~128;
                else
                    sub_data |= 128;
                break;
            case 1:
                if ((sub_data/8)%16 == 0)
                    sub_data += 15*8;
                else
                    sub_data -= 8;
                break;
            case 2:
                if (sub_data % 8 == 0)
                    sub_data += 7;
                else
                    --sub_data;
                break;
        }
        palette_block_data[128+block_index] &= ~(255<<swapping[block_cursor/3]);
        palette_block_data[128+block_index] |= sub_data<<swapping[block_cursor/3];
        gamepad_press_wait = GAMEPAD_PRESS_WAIT+GAMEPAD_PRESS_WAIT/2;
        return;
    }

    if (GAMEPAD_PRESS(0, X))
    {
        game_message[0] = 0;
        // copy or uncopy
        if (block_copying < 128)
            block_copying = 128;
        else
            block_copying = block_index;
        return;
    }
    int save_or_load = 0;
    if (GAMEPAD_PRESS(0, A)) // save
        save_or_load = 1;
    else if (GAMEPAD_PRESS(0, B)) // load
        save_or_load = 2;
    if (save_or_load)
    {
        if (block_copying < 128)
        {
            // or cancel a copy
            block_copying = 128;
            return;
        }
            
        FileError error;
        if (save_or_load == 1) // save
            error = io_save_block(block_index);
        else // load
            error = io_load_block(block_index);
        io_message_from_error(game_message, error, save_or_load);
        // TODO:
        // double tap to save all/load all
        return;
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        if (block_copying < 128)
        {
            // paste
            if (block_copying == block_index)
            {
                strcpy((char *)game_message, "pasting to same thing");
                block_copying = 128;
                return;
            }
            palette_block_data[128+block_index] = palette_block_data[128+block_copying];
            block_copying = 128;
            strcpy((char *)game_message, "pasted.");
        }
        else
        {
            previous_visual_mode = EditBlock;
            game_switch(ChooseFilename);
        }
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        block_copying = 128;
        game_message[0] = 0;
        previous_visual_mode = None;
        game_switch(EditPalette);
        return;
    }
    if (GAMEPAD_PRESS(0, start))
    {
        block_copying = 128;
        game_message[0] = 0;
        if (previous_visual_mode)
        {
            game_switch(previous_visual_mode);
            previous_visual_mode = None;
        }
        else
        {
            previous_visual_mode = EditBlock;
            view_block = palette_block_data[128+block_index];
            game_switch(View);
        }
        return;
    }
}
