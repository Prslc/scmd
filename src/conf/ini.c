#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ini.h"
#include "log.h"

static char *trim(char *str) {
    while (isspace((unsigned char)*str))
        str++;
    if (*str == '\0')
        return str;
    char *end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end))
        end--;
    end[1] = '\0';
    return str;
}

static int clamp_int(int val, int lo, int hi, const char *key) {
    if (val < lo) {
        SCMD_WARN("%s=%d below minimum, clamping to %d", key, val, lo);
        return lo;
    }
    if (val > hi) {
        SCMD_WARN("%s=%d above maximum, clamping to %d", key, val, hi);
        return hi;
    }
    return val;
}

static void str_copy(char *dst, const char *src, size_t size) {
    if (size == 0)
        return;
    size_t max_len = size - 1;
    strncpy(dst, src, max_len);
    dst[max_len] = '\0';
}

bool load_config(const char *path, Config *cfg) {
    FILE *f = fopen(path, "r");
    if (!f)
        return false;

    cfg->stop_capacity = DEFAULT_STOP_CAPACITY;
    cfg->resume_capacity = DEFAULT_RESUME_CAPACITY;
    cfg->trickle = true;
    cfg->debug = false;
    str_copy(cfg->control_path,  DEFAULT_CONTROL_PATH,  sizeof(cfg->control_path));
    str_copy(cfg->capacity_path, DEFAULT_CAPACITY_PATH, sizeof(cfg->capacity_path));

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        char *p = trim(line);

        if (*p == '\0' || *p == '#' || *p == ';' || *p == '[')
            continue;

        char *eq = strchr(p, '=');
        if (!eq)
            continue;
        *eq = '\0';

        char *key = trim(p);
        char *val = trim(eq + 1);

        if (strcasecmp(key, "stop_capacity") == 0) {
            cfg->stop_capacity = clamp_int(atoi(val), 0, 100, key);
        } else if (strcasecmp(key, "resume_capacity") == 0) {
            cfg->resume_capacity = clamp_int(atoi(val), 0, 100, key);
        } else if (strcasecmp(key, "control_path") == 0) {
            str_copy(cfg->control_path, val, sizeof(cfg->control_path));
        } else if (strcasecmp(key, "capacity_path") == 0) {
            str_copy(cfg->capacity_path, val, sizeof(cfg->capacity_path));
        } else if (strcasecmp(key, "trickle") == 0) {
            cfg->trickle =
                (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
        } else if (strcasecmp(key, "debug") == 0) {
            cfg->debug =
                (strcasecmp(val, "true") == 0 || strcmp(val, "1") == 0);
        }
    }

    fclose(f);

    // stop > resume is required; equal values cause oscillation at the boundary
    if (cfg->stop_capacity <= cfg->resume_capacity) {
        SCMD_WARN("stop_capacity (%d) <= resume_capacity (%d), resetting to defaults",
                  cfg->stop_capacity, cfg->resume_capacity);
        cfg->stop_capacity = DEFAULT_STOP_CAPACITY;
        cfg->resume_capacity = DEFAULT_RESUME_CAPACITY;
    }

    SCMD_INFO("config loaded: stop=%d resume=%d trickle=%s debug=%s control=%s capacity=%s",
              cfg->stop_capacity, cfg->resume_capacity,
              cfg->trickle ? "true" : "false", cfg->debug ? "true" : "false",
              cfg->control_path, cfg->capacity_path);
    return true;
}
