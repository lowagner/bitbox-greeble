#ifndef IO_H
#define IO_H

#include <stdint.h>

typedef enum {
    NoError = 0,
    MountError,
    ConstraintError,
    OpenError,
    ReadError,
    WriteError,
    NoDataError,
    MissingDataError,
    BotchedIt
} FileError;

FileError io_init();
FileError io_set_recent_filename();
FileError io_get_recent_filename();
void io_message_from_error(uint8_t *msg, FileError error, int save_not_load);

FileError io_save_palette(unsigned int i);
FileError io_load_palette(unsigned int i);
FileError io_save_block(unsigned int i);
FileError io_load_block(unsigned int i);
FileError io_save_instrument(unsigned int i);
FileError io_load_instrument(unsigned int i);
FileError io_save_verse(unsigned int i);
FileError io_load_verse(unsigned int i);
FileError io_save_anthem();
FileError io_load_anthem();

#endif
