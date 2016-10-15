#include "bitbox.h"
#include "common.h"
#include "font.h"
#include "name.h"
#include "io.h"

#include <string.h> // memset

#define BG_COLOR 192 // a uint8_t, uint16_t color is (BG_COLOR)|(BG_COLOR<<8)

uint8_t save_only CCM_MEMORY; // 0 - everything, 1 - palette, 2 - blocks, 3 - music

#define NUMBER_LINES 21
   
void save_init()
{
    save_only = 0;
}

void save_line()
{
    if (vga_line < 22)
    {
        if (vga_line/2 == 0)
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }
    else if (vga_line >= 22 + NUMBER_LINES*10)
    {
        if (vga_line == (22 + NUMBER_LINES*10))
            memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
        return;
    }
    int line = (vga_line-22) / 10;
    int internal_line = (vga_line-22) % 10;
    if (internal_line == 0 || internal_line == 9)
    {
        memset(draw_buffer, BG_COLOR, 2*SCREEN_W);
    }
    else
    {
        --internal_line;
        switch (line)
        {
        case 0:
            font_render_line_doubled((const uint8_t *)"greeble", 16, internal_line, 65535, BG_COLOR*257);
            break;
        case 2:
            font_render_line_doubled((const uint8_t *)"file:", 16, internal_line, 65535, BG_COLOR*257);
            font_render_line_doubled((uint8_t *)base_filename, 16+6*9, internal_line, 65535, BG_COLOR*257);
            break;
        case 3:
            font_render_line_doubled((const uint8_t *)"L/R:selective save/load", 16, internal_line, 65535, BG_COLOR*257);
            switch (save_only)
            {
            case 0:
                font_render_line_doubled((const uint8_t *)"all", 16+24*9, internal_line, 65535, BG_COLOR*257);
                break;
            case 1:
                font_render_line_doubled((const uint8_t *)"palette", 16+24*9, internal_line, 65535, BG_COLOR*257);
                break;
            case 2:
                font_render_line_doubled((const uint8_t *)"blocks", 16+24*9, internal_line, 65535, BG_COLOR*257);
                break;
            case 3:
                font_render_line_doubled((const uint8_t *)"music", 16+24*9, internal_line, 65535, BG_COLOR*257);
                break;
            }
            break;
        case 4:
            font_render_line_doubled((const uint8_t *)"  A:save to file", 16, internal_line, 65535, BG_COLOR*257);
            break;
        case 5:
            font_render_line_doubled((const uint8_t *)"  B:load from file", 16, internal_line, 65535, BG_COLOR*257);
            break;
        case 6:
            font_render_line_doubled((const uint8_t *)"  Y:choose filename", 16, internal_line, 65535, BG_COLOR*257);
            break;
        case 8:
            switch (save_only)
            {
            case 0:
                font_render_line_doubled((const uint8_t *)"start:view block", 16, internal_line, 65535, BG_COLOR*257);
                break;
            case 1:
                font_render_line_doubled((const uint8_t *)"start:edit palette", 16, internal_line, 65535, BG_COLOR*257);
                break;
            case 2:
                font_render_line_doubled((const uint8_t *)"start:edit blocks", 16, internal_line, 65535, BG_COLOR*257);
                break;
            case 3:
                font_render_line_doubled((const uint8_t *)"start:edit music", 16, internal_line, 65535, BG_COLOR*257);
                break;
            }
            break;
        case 9:
            if (save_only)
                font_render_line_doubled((uint8_t *)"select:view block", 16, internal_line, 65535, BG_COLOR*257);
            else
                font_render_line_doubled((uint8_t *)"select:\"  \"", 16, internal_line, 65535, BG_COLOR*257);
            break;
        case (NUMBER_LINES-1):
            if (game_message[0])
                font_render_line_doubled(game_message, 32, internal_line, 65535, BG_COLOR*257);
            else
                font_render_line_doubled((uint8_t *)"github.com/lowagner/bitbox-greeble", 6, internal_line, 65535, BG_COLOR*257);
            break;
        }
    }
}


void save_controls()
{
    int make_wait = 0;
    if (GAMEPAD_PRESSING(0, left))
    {
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, right))
    {
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, up))
    {
        make_wait = 1;
    }
    if (GAMEPAD_PRESSING(0, down))
    {
        make_wait = 1;
    }
    if (make_wait)
        gamepad_press_wait = GAMEPAD_PRESS_WAIT;

    int save_or_load = 0;
    if (GAMEPAD_PRESS(0, A))
        save_or_load = 1;  // save
    if (GAMEPAD_PRESS(0, B))
        save_or_load = 2; // load
    if (save_or_load)
    {
        FileError error = BotchedIt;
        int offset = 0;
        switch (save_only)
        {
        case 0: // save all
            error = (save_or_load == 1) ? io_save_palette(128) : io_load_palette(128);
            if (error != NoError)
            {
                strcpy((char *)game_message, "palette ");
                offset = 8;
                break;
            }
            error = (save_or_load == 1) ? io_save_block(128) : io_load_block(128);
            if (error != NoError)
            {
                strcpy((char *)game_message, "block ");
                offset = 6;
                break;
            }
            error = (save_or_load == 1) ? io_save_anthem() : io_load_anthem();
            if (error != NoError)
            {
                strcpy((char *)game_message, "anthem ");
                offset = 7;
                break;
            }
            error = (save_or_load == 1) ? io_save_verse(16) : io_load_verse(16);
            if (error != NoError)
            {
                strcpy((char *)game_message, "verse ");
                offset = 6;
                break;
            }
            error = (save_or_load == 1) ? io_save_instrument(16) : io_load_instrument(16);
            if (error != NoError)
            {
                strcpy((char *)game_message, "instr. ");
                offset = 7;
                break;
            }
            break;
        case 1:
            error = (save_or_load == 1) ? io_save_palette(128) : io_load_palette(128);
            if (error != NoError)
            {
                strcpy((char *)game_message, "palette ");
                offset = 8;
                break;
            }
            break;
        case 2:
            error = (save_or_load == 1) ? io_save_block(128) : io_load_block(128);
            if (error != NoError)
            {
                strcpy((char *)game_message, "block ");
                offset = 6;
                break;
            }
            break;
        case 3:
            error = (save_or_load == 1) ? io_save_anthem() : io_load_anthem();
            if (error != NoError)
            {
                strcpy((char *)game_message, "anthem ");
                offset = 7;
                break;
            }
            error = (save_or_load == 1) ? io_save_verse(16) : io_load_verse(16);
            if (error != NoError)
            {
                strcpy((char *)game_message, "verse ");
                offset = 6;
                break;
            }
            error = (save_or_load == 1) ? io_save_instrument(16) : io_load_instrument(16);
            if (error != NoError)
            {
                strcpy((char *)game_message, "instr. ");
                offset = 7;
                break;
            }
            break;
        }
        io_message_from_error(game_message+offset, error, save_or_load);
        return;
    }
    if (GAMEPAD_PRESS(0, X))
    {
        game_message[0] = 0;
        // TODO:  add functionality?
        return;
    }
    if (GAMEPAD_PRESS(0, Y))
    {
        game_message[0] = 0;
        // switch to choose name and hope to come back
        game_switch(ChooseFilename);
        previous_visual_mode = SaveLoadScreen;
        return;
    }
    if (GAMEPAD_PRESS(0, L))
    {
        game_message[0] = 0;
        save_only = (save_only - 1)&3;
        return;
    } 
    if (GAMEPAD_PRESS(0, R))
    {
        game_message[0] = 0;
        save_only = (save_only + 1)&3;
        return;
    }
    if (GAMEPAD_PRESS(0, select))
    {
        game_message[0] = 0;
        // switch to next visual mode and ignore previous_visual_mode
        game_switch(View);
        previous_visual_mode = None;
        save_only = 0; // reset save for next time
        return;
    }
    if (GAMEPAD_PRESS(0, start))
    {
        game_message[0] = 0;
        previous_visual_mode = None;
        switch (save_only)
        {
        case 0:
            game_switch(View);
            break;
        case 1:
            game_switch(EditPalette);
            break;
        case 2:
            game_switch(EditBlock);
            break;
        case 3:
            game_switch(EditAnthem);
            break;
        }
        return;
    }
}
