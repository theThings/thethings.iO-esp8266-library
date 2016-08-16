#ifndef THETHINGSIO_H
#define THETHINGSIO_H

#include "user_interface.h"

bool thethingsio_write_num(char const *, char const *, int);
bool thethingsio_write_str(char const *, char const *, char const *);

bool thethingsio_read(char const *, char const *, unsigned char, void (*callback)(void *, char *, unsigned short));

bool thethingsio_subscribe(char const *, void (*callback)(void *, char *, unsigned short));

#endif // THETHINGSIO_H
