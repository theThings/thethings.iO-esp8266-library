#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"

#include "user_config.h"
#include "user_interface.h"

#include "c_types.h"
#include "espconn.h"
#include "mem.h"

#define led 2
#define LOW 0
#define HIGH 1

void ICACHE_FLASH_ATTR network_init();

LOCAL os_timer_t network_timer;

static void ICACHE_FLASH_ATTR networkSentCb(void *arg) {
}

static void ICACHE_FLASH_ATTR networkRecvCb(void *arg, char *data, unsigned short len) {
    struct espconn *conn=(struct espconn *)arg;
    if (os_strstr(data, "\"value\":1") > 10) {
        GPIO_OUTPUT_SET(led, LOW);
        os_printf("ON \n\r");
    }
    else if (os_strstr(data, "\"value\":0") > 10) {
        GPIO_OUTPUT_SET(led, HIGH);
        os_printf("OFF \n\r");
    }
}

static void ICACHE_FLASH_ATTR networkConnectedCb(void *arg) {
    struct espconn *conn=(struct espconn *)arg;

    char data[1024];
    os_sprintf(data,
            "GET /v2/things/%s?keepAlive=%d HTTP/1.1\n"
            "Host: api.thethings.io\n"
            "Accept: application/json\n\n", TOKEN, KEEPALIVE);

    sint8 d = espconn_sent(conn, data, strlen(data));

    espconn_regist_recvcb(conn, networkRecvCb);
}

static void ICACHE_FLASH_ATTR networkReconCb(void *arg, sint8 err) {
}

static void ICACHE_FLASH_ATTR networkDisconCb(void *arg) {
}

static void ICACHE_FLASH_ATTR networkServerFoundCb(const char *name, ip_addr_t *ip, void *arg) {
    static esp_tcp tcp;
    struct espconn *conn=(struct espconn *)arg;

    if (ip == NULL) {
        os_printf("Nslookup failed: Trying again...\n");
        network_init();
    }

    conn->type=ESPCONN_TCP;
    conn->state=ESPCONN_NONE;
    conn->proto.tcp=&tcp;
    conn->proto.tcp->local_port=espconn_port();
    conn->proto.tcp->remote_port=80;
    os_memcpy(conn->proto.tcp->remote_ip, &ip->addr, 4);
    espconn_regist_connectcb(conn, networkConnectedCb);
    espconn_regist_disconcb(conn, networkDisconCb);
    espconn_regist_reconcb(conn, networkReconCb);
    espconn_regist_recvcb(conn, networkRecvCb);
    espconn_regist_sentcb(conn, networkSentCb);
    espconn_connect(conn);
}

void ICACHE_FLASH_ATTR network_start() {
    static struct espconn conn;
    static ip_addr_t ip;
    os_printf("Looking up server...\n");
    espconn_gethostbyname(&conn, "api.thethings.io", &ip, networkServerFoundCb);
}

void ICACHE_FLASH_ATTR network_check_ip(void) {
    struct ip_info ipconfig;
    os_timer_disarm(&network_timer);
    wifi_get_ip_info(STATION_IF, &ipconfig);
    if (wifi_station_get_connect_status() == STATION_GOT_IP && ipconfig.ip.addr != 0) {
        os_printf("IP found\n\r");
        network_start();
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
