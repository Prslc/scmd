#ifndef INI_H
#define INI_H

#include <stdbool.h>

#define DEFAULT_CONTROL_PATH  "/sys/class/power_supply/battery/input_suspend"
#define DEFAULT_CAPACITY_PATH "/sys/class/power_supply/battery/capacity"

#define DEFAULT_STOP_CAPACITY    80
#define DEFAULT_RESUME_CAPACITY  70

typedef struct {
    int stop_capacity;
    int resume_capacity;
    bool trickle;
    bool debug;
    char control_path[128];
    char capacity_path[128];
} Config;

bool load_config(const char *path, Config *cfg);

#endif
