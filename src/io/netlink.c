#include <linux/netlink.h>
#include <sys/socket.h>
#include <unistd.h>

int init_netlink_socket(void) {
    int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (fd < 0)
        return -1;

    struct sockaddr_nl sa = {
        .nl_family = AF_NETLINK, 
        .nl_pid = getpid(), 
        .nl_groups = 1
    };

    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        close(fd);
        return -1;
    }
    return fd;
}
