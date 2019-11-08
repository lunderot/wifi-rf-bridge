#include "rfplug.h"

uint32_t code = 0;
uint8_t wait = 0;
uint8_t send = 0;
uint8_t part = 0;
uint8_t bit = 0;

void rfplug_transmit(void *arg)
{
    if (send)
    {
        if (wait)
        {
            GPIO_OUTPUT_SET(rfplug_PORT, 0);
            wait--;
            return;
        }
        switch (part)
        {
        case 0:
            GPIO_OUTPUT_SET(rfplug_PORT, 1);
            break;
        case 1:
        case 2:
            if ((code >> bit) & 1)
                GPIO_OUTPUT_SET(rfplug_PORT, 0);
            break;
        case 3:
            GPIO_OUTPUT_SET(rfplug_PORT, 0);
            break;
        default:
            break;
        }
        part++;
        if (part == rfplug_STATES_PER_BIT)
        {
            part = 0;
            bit++;
        }
        if (bit == rfplug_BITS_PER_CODE)
        {
            bit = 0;
            wait = rfplug_PREAMBLE_LENGTH;
            send--;
        }
    }
    else {
        GPIO_OUTPUT_SET(rfplug_PORT, 0);
    }
}

void rfplug_init()
{
    hw_timer_init(0, 1);
    hw_timer_set_func(rfplug_transmit);
    hw_timer_arm(rfplug_TIME_PER_STATE);
}

void rfplug_set_code(uint32_t c)
{
    code = c;
}

void rfplug_send(uint8_t num)
{
    send = num;
}
