#include "bitbox.h"
#include "common.h"
#include "chiptune.h"
#include "instrument.h"
#include "verse.h"
#include "anthem.h"
#include "palette.h"
#include "block.h"
#include "name.h"
#include "save.h"
#include "font.h"
#include "io.h"
#include "view.h"

#include "string.h" // memcpy

VisualMode visual_mode CCM_MEMORY; 
VisualMode previous_visual_mode CCM_MEMORY;
uint16_t old_gamepad[2] CCM_MEMORY;
uint8_t gamepad_press_wait CCM_MEMORY;
uint8_t game_message[32] CCM_MEMORY;
uint32_t palette_block_data[256] CCM_MEMORY;

const uint8_t hex[64] = { 
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', // standard hex
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', // up to 32
    'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', // up to 48
    'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 138, 255  // up to 64
};

#define BSOD 140

void game_init()
{ 
    visual_mode = None;
    
    font_init();
    palette_init();
    anthem_init();
    verse_init();
    chip_init();
    instrument_init();
    view_init();
    block_init();

    // now load everything else
    if (io_get_recent_filename())
    {
        message("resetting everything\n");
        // had troubles loading a filename
        base_filename[0] = 'T';
        base_filename[1] = 'M';
        base_filename[2] = 'P';
        base_filename[3] = 0;

        // need to reset everything
        palette_reset();
        block_reset();
        anthem_reset();
        verse_reset();
        instrument_reset();
    }
    else // there was a filename to look into
    {
        if (io_load_palette(128))
            // had troubles loading a palette
            palette_reset();
        if (io_load_block(128))
            block_reset();
        if (io_load_anthem())
            anthem_reset();
        if (io_load_verse(16))
            verse_reset();
        if (io_load_instrument(16))
            instrument_reset();
    }

    // init game mode
    previous_visual_mode = None;
    game_switch(SaveLoadScreen);
}

void game_frame()
{
    kbd_emulate_gamepad();
    switch (visual_mode)
    {
    case EditPalette:
        palette_controls();
        break;
    case EditBlock:
        block_controls();
        break;
    case SaveLoadScreen:
        save_controls();
        break;
    case View:
        view_controls();
        break;
    case ChooseFilename:
        name_controls();
        break;
    case EditAnthem:
        anthem_controls();
        break;
    case EditVerse:
        verse_controls();
        break;
    case EditInstrument:
        instrument_controls();
        break;
    default:
        if (GAMEPAD_PRESS(0, select))
            game_switch(SaveLoadScreen);
        break;
    }
    
    old_gamepad[0] = gamepad_buttons[0];
    old_gamepad[1] = gamepad_buttons[1];

    if (gamepad_press_wait)
        --gamepad_press_wait;
}

void graph_frame() 
{
}

void graph_line() 
{
    if (vga_odd)
        return;
    switch (visual_mode)
    {
        case SaveLoadScreen:
            save_line();
            break;
        case View:
            view_line();
            break;
        case EditPalette:
            palette_line();
            break;
        case EditBlock:
            block_line();
            break;
        case ChooseFilename:
            name_line();
            break;
        case EditAnthem:
            anthem_line();
            break;
        case EditVerse:
            verse_line();
            break;
        case EditInstrument:
            instrument_line();
            break;
        default:
        {
            int line = vga_line/10;
            int internal_line = vga_line%10;
            if (vga_line/2 == 0 || (internal_line/2 == 4))
            {
                memset(draw_buffer, BSOD, 2*SCREEN_W);
                return;
            }
            if (line >= 4 && line < 20)
            {
                line -= 4;
                uint32_t *dst = (uint32_t *)draw_buffer + 37;
                uint32_t color_choice[2] = { (BSOD*257)|((BSOD*257)<<16), 65535|(65535<<16) };
                int shift = ((internal_line/2))*4;
                for (int c=0; c<16; ++c)
                {
                    uint8_t row = (font[c+line*16] >> shift) & 15;
                    for (int j=0; j<4; ++j)
                    {
                        *(++dst) = color_choice[row&1];
                        row >>= 1;
                    }
                    *(++dst) = color_choice[0];
                }
                return;
            }
            break;
        }
    }
}

void game_switch(VisualMode new_visual_mode)
{
    message("changing to view mode %d\n", new_visual_mode);
    if (new_visual_mode == visual_mode)
        return;

    chip_kill();

    switch (new_visual_mode)
    {
    case View:
        message("got block %d, %d, %d, %d\n", view_block&255, (view_block>>8)&255, (view_block>>16)&255, view_block>>24);
        chip_play_init(0);
        break;
    default:
        break;
    }
    
    visual_mode = new_visual_mode;
}
