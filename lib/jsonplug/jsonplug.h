#ifndef JSONPLUG_H
#define JSONPLUG_H

#include "ets_sys.h"

#define JSONPLUG_NAME_LEN 64

struct jsonplug_plug
{
    char name[JSONPLUG_NAME_LEN];
    uint32_t code;
    bool state;
};

#define JSONPLUG_SUCCESS 0
#define JSONPLUG_ERROR 1

void ICACHE_FLASH_ATTR jsonplug_parse(const char* json, size_t length, struct jsonplug_plug* plug);
void ICACHE_FLASH_ATTR jsonplug_write(struct jsonplug_plug* plug);


#endif