#include "jsonplug.h"
#include "json/jsonparse.h"
#include "osapi.h"

void ICACHE_FLASH_ATTR jsonplug_parse(const char *json, size_t length, struct jsonplug_plug *plug)
{
    char key[32] = {0};
    char value[32] = {0};

    struct jsonparse_state state;
    jsonparse_setup(&state, json, length);

    int type = JSON_TYPE_ERROR;
    while ((type = jsonparse_next(&state)) != JSON_TYPE_ERROR)
    {
        switch (type)
        {
        case JSON_TYPE_PAIR_NAME:
            jsonparse_copy_value(&state, key, 32);
            break;
        case JSON_TYPE_STRING:
            if(!os_strcmp(key, "name"))
                jsonparse_copy_value(&state, plug->name, JSONPLUG_NAME_LEN);
            break;
        case JSON_TYPE_NUMBER:
            if(!os_strcmp(key, "code"))
            {
                plug->code =  jsonparse_get_value_as_ulong(&state);
            }
            else if(!os_strcmp(key, "state"))
            {
                plug->state =  jsonparse_get_value_as_ulong(&state);
            }
            break;
        default:
            break;
        }
    }
}

void ICACHE_FLASH_ATTR jsonplug_write(struct jsonplug_plug *plug)
{
}