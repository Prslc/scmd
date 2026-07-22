#ifndef UTILS_H
#define UTILS_H

#include <stdbool.h>
#include <stddef.h>

bool write_sysfs_int(const char *path, int value);
int  parse_uevent_capacity(const char *buf, size_t len);
bool uevent_is_full(const char *buf, size_t len);
int  read_sysfs_capacity(const char *path);

#endif
