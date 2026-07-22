#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

int init_config_inotify(const char *config_path, int *wd) {
    *wd = -1;

    // Watch the parent directory so tmpfile+rename saves
    // don't invalidate the watch — directory inode never changes.
    char *dir_path = strdup(config_path);
    if (!dir_path)
        return -1;

    char *slash = strrchr(dir_path, '/');
    if (slash) {
        *slash = '\0';
        if (*dir_path == '\0')
            dir_path[0] = '/';
    } else {
        dir_path[0] = '.';
        dir_path[1] = '\0';
    }

    int fd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    if (fd < 0) {
        free(dir_path);
        return -1;
    }

    *wd = inotify_add_watch(fd, dir_path,
                            IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE);
    free(dir_path);

    if (*wd < 0) {
        close(fd);
        return -1;
    }
    return fd;
}
