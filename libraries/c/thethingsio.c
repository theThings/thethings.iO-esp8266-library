#include "thethingsio.h"

#include "osapi.h"
#include "espconn.h"
#include "mem.h"

#define QUEUE_SIZE 5

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

ip_addr_t thethingsio_ip[QUEUE_SIZE];
struct TTRequest TTQueue[QUEUE_SIZE];
struct espconn thethingsio_conn[QUEUE_SIZE];
esp_tcp tcp[QUEUE_SIZE];
unsigned short front = 0, back = 0;

static void ICACHE_FLASH_ATTR thethingsio_send_write(void *arg) {

    char payload[512];
    switch (TTQueue[back].type) {
        case WRITE_NUM:
            os_sprintf(payload, "{\"values\":[{\"key\":\"%s\",\"value\":%d}]}", TTQueue[back].proto.write_num.key, TTQueue[back].proto.write_num.value);
            break;
        case WRITE_STR:
            os_sprintf(payload, "{\"values\":[{\"key\":\"%s\",\"value\":\"%s\"}]}", TTQueue[back].proto.write_str.key, TTQueue[back].proto.write_str.value);
            break;
    }

    char data[1024];
    os_sprintf(data,
            "POST /v2/things/%s HTTP/1.1\n"
            "Host: api.thethings.io\n"
            "Accept: application/json\n"
            "Content-Type: application/json\n"
            "Content-Length: %d\n\n"
            "%s\n\n", TTQueue[back].token, strlen(payload), payload);

    struct espconn *conn = (struct espconn *)arg;
    sint8 d = espconn_sent(conn, data, strlen(data));

    back = (back + 1)%QUEUE_SIZE;
}

static void ICACHE_FLASH_ATTR thethingsio_send_subscribe(void *arg) {
    char data[1024];
    os_sprintf(data,
            "GET /v2/things/%s?keepAlive=%d HTTP/1.1\n"
            "Host: api.thethings.io\n"
            "Accept: application/json\n\n", TTQueue[back].token, KEEPALIVE);

    struct espconn *conn = (struct espconn *)arg;
    espconn_sent(conn, data, strlen(data));

    back = (back + 1)%QUEUE_SIZE;
}

static void ICACHE_FLASH_ATTR thethingsio_recv(void *arg, char *data, unsigned short size) {
    os_printf("%s", data);
}

static void ICACHE_FLASH_ATTR thethingsio_send_read(void *arg) {
    char data[1024];
    os_sprintf(data,
            "GET /v2/things/%s/resources/%s?limit=%d HTTP/1.1\n"
            "Host: api.thethings.io\n"
            "Accept: application/json\n\n", TTQueue[back].token, TTQueue[back].proto.read.key, TTQueue[back].proto.read.limit, KEEPALIVE);

    struct espconn *conn = (struct espconn *)arg;
    espconn_sent(conn, data, strlen(data));

    back = (back + 1)%QUEUE_SIZE;
}

static void ICACHE_FLASH_ATTR thethingsio_server_found(const char *name, ip_addr_t *ip, void *arg) {
    if (ip == NULL) {
        os_printf("nslookup failed.\n");
        return;
    }

    struct espconn *conn = (struct espconn *)arg;
    conn->type = ESPCONN_TCP;
    conn->state = ESPCONN_NONE;
    conn->proto.tcp = &tcp[back];
    conn->proto.tcp->local_port = espconn_port();
    conn->proto.tcp->remote_port = 80;
    os_memcpy(conn->proto.tcp->remote_ip, &ip->addr, 4);

    switch (TTQueue[back].type) {
        case WRITE_NUM:
        case WRITE_STR:
            espconn_regist_connectcb(conn, thethingsio_send_write);
            espconn_regist_recvcb(conn, thethingsio_recv);
            break;
        case SUBSCRIBE:
            espconn_regist_connectcb(conn, thethingsio_send_subscribe);
            espconn_regist_recvcb(conn, TTQueue[back].proto.subscribe.callback);
            break;
        case READ:
            espconn_regist_connectcb(conn, thethingsio_send_read);
            espconn_regist_recvcb(conn, TTQueue[back].proto.read.callback);
            break;
    }

    espconn_connect(conn);
}

bool thethingsio_write_num(char const *token, char const *key, int value) {
    TTQueue[front].type = WRITE_NUM;
    os_sprintf(TTQueue[front].proto.write_num.key, "%s", key);
    os_sprintf(TTQueue[front].token, "%s", token);
    TTQueue[front].proto.write_num.value = value;
    espconn_gethostbyname(&thethingsio_conn[front], "api.thethings.io", &thethingsio_ip[front], thethingsio_server_found);
    front = (front + 1)%QUEUE_SIZE;
}

bool thethingsio_write_str(char const *token, char const *key, char const *value) {
    TTQueue[front].type = WRITE_STR;
    os_sprintf(TTQueue[front].proto.write_str.key, "%s", key);
    os_sprintf(TTQueue[front].token, "%s", token);
    os_sprintf(TTQueue[front].proto.write_str.value, "%s", value);
    espconn_gethostbyname(&thethingsio_conn[front], "api.thethings.io", &thethingsio_ip[front], thethingsio_server_found);
    front = (front + 1)%QUEUE_SIZE;
}

void thethingsio_subscribe(char const *token, void (*callback)(void *, char *, unsigned short)) {
    TTQueue[front].type = SUBSCRIBE;
    TTQueue[front].proto.subscribe.callback = callback;
    os_sprintf(TTQueue[front].token, "%s", token);
    espconn_gethostbyname(&thethingsio_conn[front], "api.thethings.io", &thethingsio_ip[front], thethingsio_server_found);
    front = (front + 1)%QUEUE_SIZE;
}

void thethingsio_read(char const *token, char const *key, unsigned char limit, void (*callback)(void *, char *, unsigned short)) {
    TTQueue[front].type = READ;
    TTQueue[front].proto.read.limit = limit;
    TTQueue[front].proto.read.callback = callback;
    os_sprintf(TTQueue[front].proto.read.key, "%s", key);
    os_sprintf(TTQueue[front].token, "%s", token);
    espconn_gethostbyname(&thethingsio_conn[front], "api.thethings.io", &thethingsio_ip[front], thethingsio_server_found);
    front = (front + 1)%QUEUE_SIZE;
}

