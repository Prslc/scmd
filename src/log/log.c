#include <stdarg.h>
#include <stdio.h>
#include <time.h>

#include "log.h"

static FILE *log_file = NULL;

void log_init(const char *path) {
    log_file = fopen(path, "a");
    if (!log_file)
        return;
    setvbuf(log_file, NULL, _IOLBF, 0);
}

void log_close(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void scmd_log(const char *level, const char *fmt, ...) {
    if (!log_file)
        return;

    time_t now = time(NULL);
    struct tm tm;
    localtime_r(&now, &tm);
    fprintf(log_file, "%04d-%02d-%02d %02d:%02d:%02d [%s] ",
            1900 + tm.tm_year, 1 + tm.tm_mon, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec, level);

    va_list ap;
    va_start(ap, fmt);
    vfprintf(log_file, fmt, ap);
    va_end(ap);

    fputc('\n', log_file);
    fflush(log_file);
}
