#ifndef __GB_SERIAL_H
#define __GB_SERIAL_H
#include "gb_common.h"

/* Serial link port.
 *
 * Nothing is connected to the link, so a requested transfer completes
 * immediately: the outgoing byte is recorded, the line reads back as 0xFF, the
 * start bit clears and the transfer-complete interrupt is raised.
 *
 * The recorded bytes are what the blargg test ROMs use to report their
 * results, so gb_serial_text() is how a harness reads them.
 */

#define GB_SERIAL_CAPACITY 4096

void gb_serial_init(void);

/* Called by the memory map when a transfer is started. */
void gb_serial_transfer(void);

/* NUL-terminated text written to the port so far, oldest first. */
const char *gb_serial_text(void);
unsigned int gb_serial_length(void);

#endif
