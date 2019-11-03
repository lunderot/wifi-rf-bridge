#ifndef RFPLUG_H
#define RFPLUG_H

#include "ets_sys.h"
#include "gpio.h"

#define rfplug_TIME_PER_STATE 350
#define rfplug_STATES_PER_BIT 4
#define rfplug_BITS_PER_CODE 25
#define rfplug_PREAMBLE_LENGTH 30
#define rfplug_PORT 2

#define rfplug_expand(a) (\
    (0b1010101010           ) | \
    (0b0100000000 & (a << 4)) | \
    (0b0001000000 & (a << 3)) | \
    (0b0000010000 & (a << 2)) | \
    (0b0000000100 & (a << 1)) | \
    (0b0000000001 & (a << 0))   \
)

#define rfplug_expand_state(a) ( \
    (0b10101) | \
    (0b01000 & ( a << 3)) | \
    (0b00010 & (~a << 1)) \
)

#define rfplug_generate_code(main_id, sub_id, state) (\
    (rfplug_expand(main_id) << 15) | \
    (rfplug_expand(sub_id)  << 5 ) | \
    (rfplug_expand_state(state != 0) << 0) \
)

void rfplug_transmit(void *arg);
void rfplug_init();
void rfplug_set_code(uint32_t code);
void rfplug_send(uint8_t num);

#endif