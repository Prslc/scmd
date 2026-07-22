#ifndef CONTEXT_H
#define CONTEXT_H

#include "ini.h"

typedef struct {
    Config config;
    int epoll_fd;
    int nl_fd;
    int inotify_fd;
    int inotify_wd;
    bool is_suspend;
} Context;

bool init_context(Context *ctx, const char *config_path);
void cleanup_context(Context *ctx);
void event_loop(Context *ctx, const char *config_path);

#endif
