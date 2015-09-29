#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"

#include "user_config.h"

#include "thethingsio.h"

#define led 2
#define LOW 0
#define HIGH 1

void ICACHE_FLASH_ATTR network_init();

LOCAL os_timer_t network_timer;
LOCAL os_timer_t subscribe_timer;

static void ICACHE_FLASH_ATTR networkSentCb(void *arg) {
}

void ICACHE_FLASH_ATTR user_rcv(void *arg, char *data, unsigned short len) {
    if (os_strstr(data, "\"value\":1") > 10) {
        GPIO_OUTPUT_SET(led, LOW);
        os_printf("ON \n\r");
    }
    else if (os_strstr(data, "\"value\":0") > 10) {
        GPIO_OUTPUT_SET(led, HIGH);
        os_printf("OFF \n\r");
    }
}

void ICACHE_FLASH_ATTR subscribe_timer_callback(void) {
    if (!thethingsio_subscribe(TOKEN, user_rcv)) {
        os_timer_setfn(&subscribe_timer, (os_timer_func_t *)subscribe_timer_callback, NULL);
        os_timer_arm(&subscribe_timer, 1000, 0);
    }
}

void ICACHE_FLASH_ATTR network_check_ip(void) {
    struct ip_info ipconfig;
    os_timer_disarm(&network_timer);
    wifi_get_ip_info(STATION_IF, &ipconfig);
    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
        os_printf("IP found\n\r");
        thethingsio_read(TOKEN, "led", 1, user_rcv);
        os_timer_disarm(&subscribe_timer);
        os_timer_setfn(&subscribe_timer, (os_timer_func_t *)subscribe_timer_callback, NULL);
        os_timer_arm(&subscribe_timer, 1000, 0);
    } else {
        os_printf("No IP found\n\r");
        os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
        os_timer_arm(&network_timer, 1000, 0);
    }
}

void ICACHE_FLASH_ATTR network_init() {
    os_timer_disarm(&network_timer);
    os_timer_setfn(&network_timer, (os_timer_func_t *)network_check_ip, NULL);
    os_timer_arm(&network_timer, 1000, 0);
}

void ICACHE_FLASH_ATTR user_init() {
    // Set GPIO2 to output mode, and to low
    gpio_init();
    GPIO_OUTPUT_SET(led, LOW);

    // Set serial baud rate to 9600
    uart_div_modify(0, UART_CLK_FREQ/9600);

    // Set AP settings
    char ssid[32] = SSID;
    char password[64] = SSID_PASSWORD;
    struct station_config config;
    os_memcpy(&config.ssid, ssid, 32);
    os_memcpy(&config.password, password, 64);

    // Configure wifi
    wifi_set_opmode(0x1);
    wifi_station_set_config(&config);

    network_init();
}
