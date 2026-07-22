#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "log.h"
#include "utils.h"

bool write_sysfs_int(const char *path, int value) {
    int fd = open(path, O_WRONLY);
    if (fd < 0) {
        SCMD_ERR("open(%s) for write failed: %m", path);
        return false;
    }

    char buf[16];
    int len = snprintf(buf, sizeof(buf), "%d\n", value);

    if (len < 0 || (size_t)len >= sizeof(buf)) {
        SCMD_ERR("snprintf overflow writing %d to %s", value, path);
        close(fd);
        return false;
    }

    ssize_t written = write(fd, buf, (size_t)len);
    close(fd);

    if (written != len) {
        SCMD_ERR("write(%s, %d): wrote %zd of %d bytes: %m",
                 path, value, written, len);
        return false;
    }
    return true;
}

int parse_uevent_capacity(const char *buf, size_t len) {
    const char *ptr = buf;

    while (ptr < buf + len) {
        if (strncmp(ptr, "POWER_SUPPLY_CAPACITY=", 22) == 0)
            return atoi(ptr + 22);

        // strnlen guards against a missing NUL on the final field
        size_t remaining = (size_t)(buf + len - ptr);
        size_t step = strnlen(ptr, remaining);
        if (step == remaining)
            break;
        ptr += step + 1;
    }
    return -1;
}

bool uevent_is_full(const char *buf, size_t len) {
    const char *ptr = buf;
    while (ptr < buf + len) {
        if (strncmp(ptr, "POWER_SUPPLY_STATUS=Full", 24) == 0)
            return true;
        size_t remaining = (size_t)(buf + len - ptr);
        size_t step = strnlen(ptr, remaining);
        if (step == remaining)
            break;
        ptr += step + 1;
    }
    return false;
}

int read_sysfs_capacity(const char *path) {
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        SCMD_ERR("open(%s) for read failed: %m", path);
        return -1;
    }

    char buf[16] = {0};
    ssize_t len = read(fd, buf, sizeof(buf) - 1);
    close(fd);

    if (len <= 0) {
        SCMD_ERR("read(%s) failed: %m", path);
        return -1;
    }
    return atoi(buf);
}
