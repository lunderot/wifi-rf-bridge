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
    (0b0101010101           ) | \
    (0b0000000010 & (a >> 3)) | \
    (0b0000001000 & (a     )) | \
    (0b0000100000 & (a << 3)) | \
    (0b0010000000 & (a << 6)) | \
    (0b1000000000 & (a << 9))   \
)

#define rfplug_expand_state(a) ( \
    (0b10101) | \
    (0b00010 & ( a << 1)) | \
    (0b01000 & (~a << 3))   \
)

#define rfplug_generate_code(main_id, sub_id, state) (\
    (rfplug_expand_state(state) << 20) | \
    (rfplug_expand(sub_id)  << 10 ) | \
    (rfplug_expand(main_id)) \
)

void ICACHE_FLASH_ATTR rfplug_init();
void ICACHE_FLASH_ATTR rfplug_send(uint32_t c, uint8_t num);

#endif