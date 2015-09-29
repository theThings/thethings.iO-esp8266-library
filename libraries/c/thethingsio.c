#include "thethingsio.h"

#include "osapi.h"
#include "espconn.h"
#include "mem.h"

#ifndef KEEPALIVE
#define KEEPALIVE 60000
#endif

enum TTRequestType {
    WRITE_STR,
    WRITE_NUM,
    SUBSCRIBE,
    READ
};

struct TTRead {
    char key[32];
    unsigned char limit;
    void (*callback)(void *, char *, unsigned short);
};

struct TTWriteNum {
    char key[32];
    int value;
};

struct TTWriteStr {
    char key[32];
    char value[128];
};

struct TTSubscribe {
    void (*callback)(void *, char *, unsigned short);
};

struct TTRequest {
    enum TTRequestType type;
    char token[48];
    union {
        struct TTRead read;
        struct TTWriteNum write_num;
        struct TTWriteStr write_str;
        struct TTSubscribe subscribe;
    } proto;
};

ip_addr_t thethingsio_ip;
struct TTRequest thethings_request;
struct espconn thethingsio_conn;
esp_tcp tcp;
bool occupied = false;

static void ICACHE_FLASH_ATTR thethingsio_send_write(void *arg) {

    char payload[512];
    switch (thethings_request.type) {
        case WRITE_NUM:
            os_sprintf(payload, "{\"values\":[{\"key\":\"%s\",\"value\":%d}]}", thethings_request.proto.write_num.key, thethings_request.proto.write_num.value);
            break;
        case WRITE_STR:
            os_sprintf(payload, "{\"values\":[{\"key\":\"%s\",\"value\":\"%s\"}]}", thethings_request.proto.write_str.key, thethings_request.proto.write_str.value);
            break;
    }

    char data[1024];
    os_sprintf(data,
            "POST /v2/things/%s HTTP/1.1\n"
            "Host: api.thethings.io\n"
            "Accept: application/json\n"
            "Content-Type: application/json\n"
            "Content-Length: %d\n\n"
            "%s\n\n", thethings_request.token, strlen(payload), payload);

    struct espconn *conn = (struct espconn *)arg;
    sint8 d = espconn_sent(conn, data, strlen(data));

    espconn_disconnect(conn);
    occupied = false;
}

static void ICACHE_FLASH_ATTR thethingsio_send_subscribe(void *arg) {
    char data[1024];
    os_sprintf(data,
            "GET /v2/things/%s?keepAlive=%d HTTP/1.1\n"
            "Host: api.thethings.io\n"
            "Accept: application/json\n\n", thethings_request.token, KEEPALIVE);

    struct espconn *conn = (struct espconn *)arg;
    espconn_sent(conn, data, strlen(data));

    occupied = false;
}

static void ICACHE_FLASH_ATTR thethingsio_send_read(void *arg) {
    char data[1024];
    os_sprintf(data,
            "GET /v2/things/%s/resources/%s?limit=%d HTTP/1.1\n"
            "Host: api.thethings.io\n"
            "Accept: application/json\n\n", thethings_request.token, thethings_request.proto.read.key, thethings_request.proto.read.limit, KEEPALIVE);

    struct espconn *conn = (struct espconn *)arg;
    espconn_sent(conn, data, strlen(data));

    occupied = false;
}

void ICACHE_FLASH_ATTR thethingsio_recv_read(void *arg, char *data, unsigned short len) {
    thethings_request.proto.read.callback(arg, data, len);
    struct espconn *conn = (struct espconn *)arg;
    espconn_disconnect(conn);
    occupied = false;
}

static void ICACHE_FLASH_ATTR thethingsio_server_found(const char *name, ip_addr_t *ip, void *arg) {
    if (ip == NULL) {
        os_printf("nslookup failed.\n");
        return;
    }

    struct espconn *conn = (struct espconn *)arg;
    conn->type = ESPCONN_TCP;
    conn->state = ESPCONN_NONE;
    conn->proto.tcp = &tcp;
    conn->proto.tcp->local_port = espconn_port();
    conn->proto.tcp->remote_port = 80;
    os_memcpy(conn->proto.tcp->remote_ip, &ip->addr, 4);

    switch (thethings_request.type) {
        case WRITE_NUM:
        case WRITE_STR:
            espconn_regist_connectcb(conn, thethingsio_send_write);
            break;
        case SUBSCRIBE:
            espconn_regist_connectcb(conn, thethingsio_send_subscribe);
            espconn_regist_recvcb(conn, thethings_request.proto.subscribe.callback);
            break;
        case READ:
            espconn_regist_connectcb(conn, thethingsio_send_read);
            espconn_regist_recvcb(conn, thethingsio_recv_read);
            break;
    }

    espconn_connect(conn);
}

bool ICACHE_FLASH_ATTR thethingsio_write_num(char const *token, char const *key, int value) {
    if (occupied) return false;
    occupied = true;
    thethings_request.type = WRITE_NUM;
    os_sprintf(thethings_request.proto.write_num.key, "%s", key);
    os_sprintf(thethings_request.token, "%s", token);
    thethings_request.proto.write_num.value = value;
    espconn_gethostbyname(&thethingsio_conn, "api.thethings.io", &thethingsio_ip, thethingsio_server_found);
    return true;
}

bool ICACHE_FLASH_ATTR thethingsio_write_str(char const *token, char const *key, char const *value) {
    if (occupied) return false;
    occupied = true;
    thethings_request.type = WRITE_STR;
    os_sprintf(thethings_request.proto.write_str.key, "%s", key);
    os_sprintf(thethings_request.token, "%s", token);
    os_sprintf(thethings_request.proto.write_str.value, "%s", value);
    espconn_gethostbyname(&thethingsio_conn, "api.thethings.io", &thethingsio_ip, thethingsio_server_found);
    return true;
}

bool ICACHE_FLASH_ATTR thethingsio_subscribe(char const *token, void (*callback)(void *, char *, unsigned short)) {
    if (occupied) return false;
    occupied = true;
    thethings_request.type = SUBSCRIBE;
    thethings_request.proto.subscribe.callback = callback;
    os_sprintf(thethings_request.token, "%s", token);
    espconn_gethostbyname(&thethingsio_conn, "api.thethings.io", &thethingsio_ip, thethingsio_server_found);
    return true;
}

bool ICACHE_FLASH_ATTR thethingsio_read(char const *token, char const *key, unsigned char limit, void (*callback)(void *, char *, unsigned short)) {
    if (occupied) return false;
    occupied = true;
    thethings_request.type = READ;
    thethings_request.proto.read.limit = limit;
    thethings_request.proto.read.callback = callback;
    os_sprintf(thethings_request.proto.read.key, "%s", key);
    os_sprintf(thethings_request.token, "%s", token);
    espconn_gethostbyname(&thethingsio_conn, "api.thethings.io", &thethingsio_ip, thethingsio_server_found);
    return true;
}

