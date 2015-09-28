# C API Reference

Before using any of these functions the esp8266 should be connected as STATION and with an IP assigned.

```
bool thethingsio_write_num(char const *token, char const *key, int value)
bool thethingsio_write_str(char const *token, char const *key, char const *value)
```

Send values to the specified token.

```
void thethingsio_subscribe(char const *token, void (*callback)(void *arg, char *data, unsigned short size))
```

Subscribe to messages from the specified token.
The first argument of the callback is the connection as a `struct espconn *`.
