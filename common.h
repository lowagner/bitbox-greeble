#ifndef COMMON_H
#define COMMON_H
#include <stdint.h>
#define SCREEN_W 320
#define SCREEN_H 240

extern uint16_t palette[16];

#define GAMEPAD_PRESS_WAIT 8

typedef enum {
    None=0,
    View,
    EditBlock,
    EditPalette,
    EditAnthem,
    EditVerse,
    EditInstrument,
    SaveLoadScreen,
    ChooseFilename
} VisualMode;

extern VisualMode visual_mode;
extern VisualMode previous_visual_mode;

#define GAMEPAD_PRESS(id, key) ((gamepad_buttons[id]) & (~old_gamepad[id]) & (gamepad_##key))
#define GAMEPAD_PRESSING(id, key) ((gamepad_buttons[id]) & (gamepad_##key) & (~old_gamepad[id] | ((gamepad_press_wait == 0)*gamepad_##key)))
extern uint16_t old_gamepad[2];
extern uint8_t gamepad_press_wait;

extern uint8_t game_message[32];
extern const uint8_t hex[64]; // not exactly hex but ok!

void game_switch(VisualMode new_visual_mode);

extern uint32_t palette_block_data[256];
extern uint32_t view_block;

uint8_t randomize(uint8_t arg);
#endif
