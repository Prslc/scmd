#include <stdbool.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <sys/socket.h>
#include <unistd.h>

#include "context.h"
#include "log.h"
#include "utils.h"

#define MAX_UEVENT_DRAIN 8

static void handle_uevent(Context *ctx, const char *buf, ssize_t len) {
    if (!strstr(buf, "power_supply") && !strstr(buf, "spmi"))
        return;

    int cap = parse_uevent_capacity(buf, (size_t)len);
    if (cap < 0)
        return;

    if (ctx->config.debug)
        SCMD_DBG("uevent: capacity=%d%% suspend=%d", cap, ctx->is_suspend);

    if (cap >= ctx->config.stop_capacity && !ctx->is_suspend) {
        // trickle only matters at stop=100: wait for charge IC to report
        // STATUS=Full so we don't cut power during the final trickle phase.
        if (ctx->config.trickle && ctx->config.stop_capacity == 100 &&
            !uevent_is_full(buf, (size_t)len)) {
            if (ctx->config.debug)
                SCMD_DBG("trickle: at %d%% but STATUS != Full, waiting", cap);
            return;
        }

        if (write_sysfs_int(ctx->config.control_path, 1)) {
            ctx->is_suspend = true;
            SCMD_INFO("uevent: suspended charging at %d%% (limit=%d%%)",
                      cap, ctx->config.stop_capacity);
        }
    } else if (cap <= ctx->config.resume_capacity && ctx->is_suspend) {
        if (write_sysfs_int(ctx->config.control_path, 0)) {
            ctx->is_suspend = false;
            SCMD_INFO("uevent: resumed charging at %d%% (limit=%d%%)",
                      cap, ctx->config.resume_capacity);
        }
    }
}

static bool handle_inotify(Context *ctx, const char *config_path,
                           const char *buf, ssize_t rd) {
    const char *bn = strrchr(config_path, '/');
    bn = bn ? bn + 1 : config_path;

    bool reloaded = false;

    for (const char *p = buf; p < buf + rd;) {
        const struct inotify_event *iev = (const struct inotify_event *)p;

        if (iev->len > 0 && strcmp(iev->name, bn) == 0) {
            if (iev->mask & (IN_CLOSE_WRITE | IN_MOVED_TO | IN_CREATE)) {
                if (ctx->config.debug)
                    SCMD_DBG("inotify: config file changed, reloading");
                Config new_cfg;
                if (load_config(config_path, &new_cfg)) {
                    ctx->config = new_cfg;
                    reloaded = true;
                } else {
                    SCMD_WARN("config reload failed for %s", config_path);
                }
            }
        }

        p += sizeof(struct inotify_event) + iev->len;
    }

    return reloaded;
}

static void sync_state(Context *ctx) {
    int cap = read_sysfs_capacity(ctx->config.capacity_path);
    if (cap < 0)
        return;

    if (cap >= ctx->config.stop_capacity && !ctx->is_suspend) {
        if (write_sysfs_int(ctx->config.control_path, 1)) {
            ctx->is_suspend = true;
            SCMD_INFO("reload: suspended charging at %d%%", cap);
        }
    } else if (cap <= ctx->config.resume_capacity && ctx->is_suspend) {
        if (write_sysfs_int(ctx->config.control_path, 0)) {
            ctx->is_suspend = false;
            SCMD_INFO("reload: resumed charging at %d%%", cap);
        }
    }
}

void event_loop(Context *ctx, const char *config_path) {
    struct epoll_event events[4];
    char buf[4096];

    SCMD_INFO("entering event loop");

    while (1) {
        int nfds = epoll_wait(ctx->epoll_fd, events, 4, -1);
        if (nfds < 0) {
            SCMD_ERR("epoll_wait: %m");
            break;
        }
        if (ctx->config.debug && nfds > 0)
            SCMD_DBG("epoll woke: %d events", nfds);

        for (int i = 0; i < nfds; i++) {
            int fd = events[i].data.fd;

            if (fd == ctx->nl_fd) {
                for (int drain = 0; drain < MAX_UEVENT_DRAIN; drain++) {
                    ssize_t len = recv(fd, buf, sizeof(buf) - 1, MSG_DONTWAIT);
                    if (len <= 0)
                        break;
                    buf[len] = '\0';
                    handle_uevent(ctx, buf, len);
                }
            } else if (fd == ctx->inotify_fd) {
                ssize_t rd = read(fd, buf, sizeof(buf));
                if (rd <= 0)
                    continue;

                if (handle_inotify(ctx, config_path, buf, rd))
                    sync_state(ctx);
            }
        }
    }

    SCMD_INFO("event loop exited");
}
