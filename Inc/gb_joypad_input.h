#ifndef __GB_JOYPAD_INPUT_H
#define __GB_JOYPAD_INPUT_H
#include "gb_common.h"

void gb_input(uint8_t keys);

/* The port as the CPU sees it: the low nibble is derived from the key state
 * and the currently selected half of the pad, so it has to be computed at the
 * moment of the read. */
uint8_t gb_joypad_read(void);

#endif