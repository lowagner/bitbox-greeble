#ifndef VIEW_H
#define VIEW_H

#include <stdint.h>

void drill_down(uint32_t *color, uint32_t *length, uint32_t block, int x, int y, int size);
void view_init();
void view_line();
void view_controls();

#endif

