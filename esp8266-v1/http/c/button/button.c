#include "thethingsio.h"
#include "user_config.h"
#include "gpio.h"
#include "osapi.h"

LOCAL os_timer_t network_timer;

void ICACHE_FLASH_ATTR network_check_ip(void) {
    struct ip_info ipconfig;
    os_timer_disarm(&network_timer);
    wifi_get_ip_info(STATION_IF, &ipconfig);
    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
        os_printf("IP found\n\r");
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

void ICACHE_FLASH_ATTR callback(uint32 interruptMask, void *arg) {
    struct ip_info ipconfig;
    wifi_get_ip_info(STATION_IF, &ipconfig);
    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
        thethingsio_write_num(TOKEN, "button", 1);
    }
}

void ICACHE_FLASH_ATTR user_init() {
    // Set GPIO2 to output mode, and to low
    gpio_init();
    gpio_pin_intr_state_set(2, GPIO_PIN_INTR_ANYEDGE);
    gpio_intr_handler_register(&callback, &callback);

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
