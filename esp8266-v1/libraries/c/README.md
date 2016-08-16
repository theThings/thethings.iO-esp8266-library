# C API Reference

Before using any of these functions the esp8266 should be connected as STATION and with an IP assigned.

Since only one simultaneous connection is possible, each function returns weather it could start the connection or not (weather it was occupied doing another task or not)

```
bool thethingsio_write_num(char const *token, char const *key, int value)
bool thethingsio_write_str(char const *token, char const *key, char const *value)
```

Send values to the specified token and key.

```
bool thethingsio_read(char const *token, char const *key, unsigned char limit, void (*callback)(void *arg, char *data, unsigned short size))
```

Read the last `limit` values of the specified key and token
The first argument of the callback is the connection passed as a `struct espconn *`.

```
bool thethingsio_subscribe(char const *token, void (*callback)(void *arg, char *data, unsigned short size))
```

Subscribe to messages from the specified token.
The callback has the same form as the `thethingsio_read` one.
