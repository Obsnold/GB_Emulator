#include "gb_serial.h"
#include "gb_mem_map.h"

#include <string.h>

static char g_serial_text[GB_SERIAL_CAPACITY + 1];
static unsigned int g_serial_length = 0;

void gb_serial_init(void){
    memset(g_serial_text, 0, sizeof(g_serial_text));
    g_serial_length = 0;
}

void gb_serial_transfer(void){
    uint8_t out = get_mem_map_8(SERIAL_SB);

    // Keep the byte for whoever is reading the port's output. Once the buffer
    // is full the rest is dropped; a test ROM's verdict arrives long before
    // 4 KiB of text does.
    if(g_serial_length < GB_SERIAL_CAPACITY){
        g_serial_text[g_serial_length] = (char)out;
        g_serial_length++;
        g_serial_text[g_serial_length] = '\0';
    }

    // With no cable attached the other end drives the line high, so the byte
    // shifted in is 0xFF.
    set_mem_map_8(SERIAL_SB, 0xFF);

    // The transfer is instantaneous here, so it is already complete: clear the
    // start bit and raise the interrupt a finished transfer would raise.
    clear_mem_map_bit(SERIAL_SC, SERIAL_SC_START);
    set_mem_map_bit(INTERRUPT_FLAGS, INTERRUPT_SERIAL);
}

const char *gb_serial_text(void){
    return g_serial_text;
}

unsigned int gb_serial_length(void){
    return g_serial_length;
}
