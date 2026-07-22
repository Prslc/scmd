#ifndef LOG_H
#define LOG_H

void log_init(const char *path);
void log_close(void);
void scmd_log(const char *level, const char *fmt, ...)
    __attribute__((format(printf, 2, 3)));

#define SCMD_INFO(fmt, ...)  scmd_log("INFO",  fmt, ##__VA_ARGS__)
#define SCMD_WARN(fmt, ...)  scmd_log("WARN",  fmt, ##__VA_ARGS__)
#define SCMD_ERR(fmt, ...)   scmd_log("ERROR", fmt, ##__VA_ARGS__)
#define SCMD_DBG(fmt, ...)   scmd_log("DEBUG", fmt, ##__VA_ARGS__)

#endif
