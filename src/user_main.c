#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "espconn.h"

#include "user_interface.h"
#include "wifi_config.h"

static os_timer_t ptimer;

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABBBCDDD
 *                A : rf cal
 *                B : at parameters
 *                C : rf init data
 *                D : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
*******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map)
    {
    case FLASH_SIZE_4M_MAP_256_256:
        rf_cal_sec = 128 - 5;
        break;

    case FLASH_SIZE_8M_MAP_512_512:
        rf_cal_sec = 256 - 5;
        break;

    case FLASH_SIZE_16M_MAP_512_512:
    case FLASH_SIZE_16M_MAP_1024_1024:
        rf_cal_sec = 512 - 5;
        break;

    case FLASH_SIZE_32M_MAP_512_512:
    case FLASH_SIZE_32M_MAP_1024_1024:
        rf_cal_sec = 1024 - 5;
        break;

    case FLASH_SIZE_64M_MAP_1024_1024:
        rf_cal_sec = 2048 - 5;
        break;
    case FLASH_SIZE_128M_MAP_1024_1024:
        rf_cal_sec = 4096 - 5;
        break;
    default:
        rf_cal_sec = 0;
        break;
    }
    return rf_cal_sec;
}

LOCAL struct espconn masterconn;

const char *msg_welcome =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: 6\r\n"
    "Content-Type: text/html\r\n"
    "Connection: Closed\r\n"
    "\r\n"
    "Hello!";

LOCAL void ICACHE_FLASH_ATTR
tcp_disconnect(void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;
    os_printf("tcp connection disconnected\n");
}

LOCAL void ICACHE_FLASH_ATTR
tcp_recv(void *arg, char *pusrdata, unsigned short length)
{
    struct espconn *pespconn = (struct espconn *)arg;
    os_printf("sending data\n");
    espconn_send(pespconn, (uint8 *)msg_welcome, os_strlen(msg_welcome));
}

LOCAL void ICACHE_FLASH_ATTR
tcp_connect(void *arg)
{
    struct espconn *pespconn = (struct espconn *)arg;

    os_printf("tcp connection established\n");

    espconn_regist_recvcb(pespconn, tcp_recv);
    espconn_regist_disconcb(pespconn, tcp_disconnect);
    // espconn_regist_reconcb(pespconn, tcpserver_recon_cb);
    // espconn_regist_sentcb(pespconn, tcpclient_sent_cb);
}

void ICACHE_FLASH_ATTR
http_init(void)
{
    masterconn.type = ESPCONN_TCP;
    masterconn.state = ESPCONN_NONE;
    masterconn.proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    masterconn.proto.tcp->local_port = 80;
    espconn_regist_connectcb(&masterconn, tcp_connect);
    espconn_accept(&masterconn);
    espconn_regist_time(&masterconn, 10, 0);
}

#define expand(a) (\
    (0b1010101010           ) | \
    (0b0100000000 & (a << 4)) | \
    (0b0001000000 & (a << 3)) | \
    (0b0000010000 & (a << 2)) | \
    (0b0000000100 & (a << 1)) | \
    (0b0000000001 & (a << 0))   \
)

#define expand_state(a) ( \
    (0b10101) | \
    (0b01000 & ( a << 3)) | \
    (0b00010 & (~a << 1)) \
)

#define code(a, b, state) (\
    (expand(a)  << 15) | \
    (expand(b)  << 5 ) | \
    (expand_state(state) << 0) \
)

#define STATES_PER_BIT 4
#define BITS_PER_CODE 25
#define PREAMBLE_LENGTH 30
#define PORT 2
static uint8_t send = 0;
static uint32_t code = code(0b10000, 0b10000, 1);

void transmit(void *arg)
{
    static uint8_t bit = 0;
    static uint8_t part = 0;
    static uint16_t wait = 0;

    if (send)
    {
        if (wait)
        {
            GPIO_OUTPUT_SET(PORT, 0);
            wait--;
            return;
        }
        switch (part)
        {
        case 0:
            GPIO_OUTPUT_SET(PORT, 1);
            break;
        case 1:
        case 2:
            if ((code >> bit) & 1)
                GPIO_OUTPUT_SET(PORT, 0);
            break;
        case 3:
            GPIO_OUTPUT_SET(PORT, 0);
            break;
        default:
            break;
        }
        part++;
        if (part == STATES_PER_BIT)
        {
            part = 0;
            bit++;
        }
        if (bit == BITS_PER_CODE)
        {
            bit = 0;
            wait = PREAMBLE_LENGTH;
            send--;
        }
    }
    else {
        GPIO_OUTPUT_SET(PORT, 0);
    }
}

void blinky(void *arg)
{
	send = 4;
}

void ICACHE_FLASH_ATTR
user_init(void)
{
    gpio_init();

    uart_init(115200, 115200);
    os_printf("SDK version:%s\n", system_get_sdk_version());

    struct station_config stationConf;

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    wifi_set_opmode(STATION_MODE);
    os_memcpy(&stationConf.ssid, ssid, 32);
    os_memcpy(&stationConf.password, password, 32);
    wifi_station_set_config(&stationConf);
    wifi_station_connect();

    os_timer_disarm(&ptimer);
    os_timer_setfn(&ptimer, (os_timer_func_t *)blinky, NULL);
    os_timer_arm(&ptimer, 5000, 1);

    hw_timer_init(0, 1);
    hw_timer_set_func(transmit);
    hw_timer_arm(350);

    http_init();
}
