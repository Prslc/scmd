#include <stdbool.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "context.h"
#include "log.h"
#include "utils.h"

int init_netlink_socket(void);
int init_config_inotify(const char *config_path, int *wd);

static void sync_state(Context *ctx) {
    int cap = read_sysfs_capacity(ctx->config.capacity_path);
    if (cap < 0)
        return;

    if (cap >= ctx->config.stop_capacity && !ctx->is_suspend) {
        if (write_sysfs_int(ctx->config.control_path, 1)) {
            ctx->is_suspend = true;
            SCMD_INFO("cold-start: suspended charging at %d%%", cap);
        }
    } else if (cap <= ctx->config.resume_capacity && ctx->is_suspend) {
        if (write_sysfs_int(ctx->config.control_path, 0)) {
            ctx->is_suspend = false;
            SCMD_INFO("cold-start: resumed charging at %d%%", cap);
        }
    }
}

bool init_context(Context *ctx, const char *config_path) {
    memset(ctx, 0, sizeof(Context));
    // -1 so cleanup on early failure doesn't close(0)
    ctx->epoll_fd = -1;
    ctx->nl_fd = -1;
    ctx->inotify_fd = -1;
    ctx->inotify_wd = -1;

    log_init("scmd.log");
    SCMD_INFO("scmd starting, config=%s", config_path);

    if (!load_config(config_path, &ctx->config)) {
        SCMD_ERR("failed to load config from %s", config_path);
        return false;
    }

    ctx->epoll_fd = epoll_create1(EPOLL_CLOEXEC);
    if (ctx->epoll_fd < 0) {
        SCMD_ERR("epoll_create1: %m");
        return false;
    }

    ctx->nl_fd = init_netlink_socket();
    if (ctx->nl_fd < 0) {
        SCMD_ERR("init_netlink_socket: %m");
        goto err_epoll;
    }

    ctx->inotify_fd = init_config_inotify(config_path, &ctx->inotify_wd);
    if (ctx->inotify_fd < 0) {
        SCMD_ERR("init_config_inotify(%s): %m", config_path);
        goto err_nl;
    }

    struct epoll_event ev = {.events = EPOLLIN};

    ev.data.fd = ctx->nl_fd;
    if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, ctx->nl_fd, &ev) < 0) {
        SCMD_ERR("epoll_ctl(nl_fd): %m");
        goto err_inotify;
    }

    ev.data.fd = ctx->inotify_fd;
    if (epoll_ctl(ctx->epoll_fd, EPOLL_CTL_ADD, ctx->inotify_fd, &ev) < 0) {
        SCMD_ERR("epoll_ctl(inotify_fd): %m");
        goto err_inotify;
    }

    sync_state(ctx);

    return true;

err_inotify:
    close(ctx->inotify_fd);
    ctx->inotify_fd = -1;
err_nl:
    close(ctx->nl_fd);
    ctx->nl_fd = -1;
err_epoll:
    close(ctx->epoll_fd);
    ctx->epoll_fd = -1;
    return false;
}

void cleanup_context(Context *ctx) {
    if (ctx->inotify_fd >= 0)
        close(ctx->inotify_fd);
    if (ctx->nl_fd >= 0)
        close(ctx->nl_fd);
    if (ctx->epoll_fd >= 0)
        close(ctx->epoll_fd);
    log_close();
}
