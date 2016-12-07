# DO NOT FORGET to define BITBOX environment variable 

USE_SDCARD = 1      # allow use of SD card for io
#DEFINES += DEBUG_CHIPTUNE

NAME = greeble
# font files need to be first in order to be generated first:
GAME_C_FILES = font.c name.c view.c io.c block.c palette.c \
    chiptune.c verse.c instrument.c anthem.c save.c main.c
GAME_H_FILES = font.h name.h view.h io.h block.h palette.h \
    chiptune.h verse.h instrument.h anthem.h save.h common.h 
DEFINES += VGA_MODE=320

# see this file for options
include $(BITBOX)/kernel/bitbox.mk

destroy:
	rm -f RECENT16.TXT *.GBL

very-clean: clean destroy
