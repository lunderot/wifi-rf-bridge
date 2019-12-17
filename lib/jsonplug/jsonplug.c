#include "jsonplug.h"
#include "json/jsonparse.h"
#include "json/jsontree.h"
#include "osapi.h"
#include "mem.h"

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
            if (!os_strcmp(key, "name"))
                jsonparse_copy_value(&state, plug->name, JSONPLUG_NAME_LEN);
            break;
        case JSON_TYPE_NUMBER:
            if (!os_strcmp(key, "code"))
            {
                plug->code = jsonparse_get_value_as_ulong(&state);
            }
            else if (!os_strcmp(key, "state"))
            {
                plug->state = jsonparse_get_value_as_ulong(&state);
            }
            break;
        default:
            break;
        }
    }
}

LOCAL char *json_buf;
LOCAL int pos;
LOCAL int size;
LOCAL struct jsonplug_plug *plugs_data;

int ICACHE_FLASH_ATTR jsonplug_get(struct jsontree_context *js_ctx)
{
    const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);
    int index = js_ctx->index[js_ctx->depth - 2];

    if (os_strncmp(path, "code", 4) == 0)
    {
        jsontree_write_int(js_ctx, plugs_data[index].code);
    }
    else if (os_strncmp(path, "name", 4) == 0)
    {
        jsontree_write_string(js_ctx, plugs_data[index].name);
    }
    else if (os_strncmp(path, "state", 5) == 0)
    {
        jsontree_write_int(js_ctx, plugs_data[index].state);
    }
    return 0;
}

int ICACHE_FLASH_ATTR
json_putchar(int c)
{
    if (json_buf != NULL && pos <= size)
    {
        json_buf[pos++] = c;
        return c;
    }

    return 0;
}

struct jsontree_callback switch_callback = JSONTREE_CALLBACK(jsonplug_get, NULL);

JSONTREE_OBJECT(plug,
                JSONTREE_PAIR("name", &switch_callback),
                JSONTREE_PAIR("code", &switch_callback),
                JSONTREE_PAIR("state", &switch_callback));

JSONTREE_ARRAY(plugs,
               JSONTREE_PAIR_ARRAY(&plug),
               JSONTREE_PAIR_ARRAY(&plug),
               JSONTREE_PAIR_ARRAY(&plug));

JSONTREE_OBJECT(main, JSONTREE_PAIR("plugs", &plugs));

void ICACHE_FLASH_ATTR jsonplug_write(struct jsonplug_plug plugs[3], char *const buffer, size_t buffer_sz)
{
    plugs_data = plugs;
    json_buf = buffer;
    size = buffer_sz;
    pos = 0;

    struct jsontree_context ctx = {0};
    jsontree_setup(&ctx, (struct jsontree_value *)&main, json_putchar);
    jsontree_reset(&ctx);

    while (jsontree_print_next(&ctx) && ctx.path <= ctx.depth)
        ;
}