#include "bitbox.h"
#include "common.h"
#include "font.h"
#include "name.h"
#include "io.h"

#include <string.h> // memset

#define BG_COLOR 0 // a uint8_t, uint16_t color is (_COLOR)|(_COLOR<<8)

int view_zoom CCM_MEMORY;
uint32_t view_block CCM_MEMORY;
int view_x CCM_MEMORY;
int view_y CCM_MEMORY;
int view_cursor CCM_MEMORY; 
int view_block_size CCM_MEMORY;

uint8_t view_history_size CCM_MEMORY;
uint8_t view_history_index CCM_MEMORY;
uint32_t view_history[255] CCM_MEMORY;

static inline uint32_t pack8(uint8_t *x)
{
    return (x[0])|(x[1]<<8)|(x[2]<<16)|(x[3]<<24);
}

static inline void unpack32(uint8_t *x, uint32_t p)
{
    x[0] = p&255;
    x[1] = (p>>8)&255;
    x[2] = (p>>16)&255;
    x[3] = p>>24;
}

void view_init()
{
    view_block = 1 | (1<<16);
    view_cursor = 1;
    view_zoom = 0;
    view_block_size = 128;
    view_x = (320-128)/2;
    view_y = (240-128)/2;
    view_history_size = 0;
    view_history_index = 255;
}

void drill_down(uint32_t *color, uint32_t *length, uint32_t block, int x, int y, int size)
{
    while (size > 8)
    {
        if (y < size/2) // top quadrants
        {
            if (x < size/2) // left top quadrant
            {
                uint8_t value = (block>>8)&255;
                if (value & 128)
                {
                    block = palette_block_data[value];
                    size /= 2;
                    continue;
                }

                *color = palette_block_data[value];
                *length = size/2 - x;
                return;
            }
            else // right top quadrant
            {
                uint8_t value = block&255;
                if (value & 128)
                {
                    block = palette_block_data[value];
                    size /= 2;
                    x -= size;
                    continue;
                }

                *color = palette_block_data[value];
                *length = size - x;
                return;
            }
        }
        else // bottom quadrants
        {
            if (x < size/2) // left bottom quadrant
            {
                uint8_t value = (block>>16)&255;
                if (value & 128)
                {
                    block = palette_block_data[value];
                    size /= 2;
                    y -= size;
                    continue;
                }
                
                *color = palette_block_data[value];
                *length = size/2 - x;
                return;
            }
            else // right bottom quadrant
            {
                uint8_t value = (block>>24)&255;
                if (value & 128)
                {
                    block = palette_block_data[value];
                    size /= 2;
                    x -= size;
                    y -= size;
                    continue;
                }
                
                *color = palette_block_data[value];
                *length = size - x;
                return;
            }
        }
    }

    if (size != 8)
        message("something wrong with drill down size %d\n", size);
    // don't recurse any lower, but just split into quadrants here and force them into colors
    if (y < 4) // top quadrants
    {
        if (x < 4) // left top quadrant
        {
            *color = palette_block_data[(block>>8)&127];
            if (*color == palette_block_data[block&127])
                *length = 8 - x;
            else
                *length = 4 - x;
        }
        else // right top quadrant
        {
            *color = palette_block_data[block&127];
            *length = 8 - x;
        }
    }
    else // bottom quadrants
    {
        if (x < 4) // left bottom quadrant
        {
            *color = palette_block_data[(block>>16)&127];
            if (*color == palette_block_data[(block>>24)&127])
                *length = 8 - x;
            else
                *length = 4 - x;
        }
        else // right bottom quadrant
        {
            *color = palette_block_data[(block>>24)&127];
            *length = 8 - x;
        }
    }
}

void view_line()
{
    if (view_y < 0)
    {
        // we're above the block
        if (vga_line/2 == 0)
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        else if (vga_line >= view_y + view_block_size)
        {
            // we're below the block
            if (vga_line < view_y + view_block_size + 2)
                memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
            return;
        }
    }
    else if (vga_line < view_y)
    {
        // we're above the block
        if (vga_line/2 == 0)
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= view_y + view_block_size)
    {
        // we're below the block
        if (vga_line < view_y + view_block_size + 2)
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }

    int ix, x;
    if (view_x < 0)
    {
        ix = 0;
        x = -view_x;
    }
    else
    {
        ix = view_x;
        x = 0;
    }
    int final_ix = view_x + view_block_size; // final image coordinate, not inclusive
    if (final_ix > 320)
        final_ix = 320;
    int y = vga_line - view_y;
    while (ix < final_ix)
    {
        uint32_t color, length;
        drill_down(&color, &length, view_block, x, y, view_block_size);
        x += length;
        int next_ix = ix + length;
        if (next_ix > final_ix)
            next_ix = final_ix;
        do
            draw_buffer[ix++] = color & 65535; // need to add bg sometime
        while (ix < next_ix);
    }
}

void view_controls()
{
    int moved = 0;
    if (GAMEPAD_PRESSING(0, R))
        ++moved;
    if (GAMEPAD_PRESSING(0, L))
        --moved;
    if (moved)
    {
        gamepad_press_wait = GAMEPAD_PRESS_WAIT+GAMEPAD_PRESS_WAIT/2;
        return;
    }
    if (GAMEPAD_PRESSING(0, right))
    {
        if (view_x + view_block_size > 1)
            --view_x;
        ++moved;
    }
    if (GAMEPAD_PRESSING(0, left))
    {
        if (view_x < 320-1)
            ++view_x;
        ++moved;
    }
    if (GAMEPAD_PRESSING(0, down))
    {
        if (view_y + view_block_size > 1)
            --view_y;
        ++moved;
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        if (view_y < 240-1)
            ++view_y;
        ++moved;
    }
    if (moved)
    {
        gamepad_press_wait = 1;
        return;
    }
    if (GAMEPAD_PRESS(0, X) || GAMEPAD_PRESS(0, B)) // backtrack
    {
        // unzoom it
        if (!view_history_size)
        {
            switch (view_block_size)
            {
                case 128:
                    return;
                case 256:
                    view_block_size = 128;
                    break;
                case 512:
                    view_block_size = 256;
                    break;
                default:
                    message("unknown size %d shouldn't happen!\n", view_block_size);
                    view_block_size = 128;
            }
            view_x *= 2;
            view_y *= 2;
            return;
        }
        view_block = view_history[view_history_index];
        if (--view_history_index == 255)
            view_history_index = 254;
        --view_history_size;
        return;
    }
    if (GAMEPAD_PRESS(0, Y) || GAMEPAD_PRESS(0, A)) // advance
    {
        // zoom it
        if (view_block_size < 512)
        {
            view_block_size *= 2; 
            view_x /= 2;
            view_y /= 2;
            return;
        }
        // put current view into history
        if (++view_history_index == 255)
            view_history_index = 0;
        if (++view_history_size == 0)
            view_history_size = 255;
        view_history[view_history_index] = view_block;
        // now update view_block
        // TODO:  update view_block
        message("zoom not yet implemented\n");
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        game_switch(EditBlock);
        return;
    }
    if (GAMEPAD_PRESS(0, start))
    {
        game_message[0] = 0;
        // TODO: show help or not
        if (previous_visual_mode)
        {
            game_switch(previous_visual_mode);
            previous_visual_mode = None;
        }
        else
            game_switch(SaveLoadScreen);
        return;
    }
}
