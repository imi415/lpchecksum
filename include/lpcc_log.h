#ifndef LPCC_LOG_H
#define LPCC_LOG_H

typedef enum {
    LPCC_LOG_DEBUG,
    LPCC_LOG_INFO,
    LPCC_LOG_WARN,
    LPCC_LOG_ERROR,
    LPCC_LOG_CRITICAL,
} lpcc_log_level_t; 

#define LPCC_LOG_DEBUG(...)    lpcc_logprintf(LPCC_LOG_DEBUG, __VA_ARGS__)
#define LPCC_LOG_INFO(...)     lpcc_logprintf(LPCC_LOG_INFO, __VA_ARGS__)
#define LPCC_LOG_WARN(...)     lpcc_logprintf(LPCC_LOG_WARN, __VA_ARGS__)
#define LPCC_LOG_ERROR(...)    lpcc_logprintf(LPCC_LOG_ERROR, __VA_ARGS__)
#define LPCC_LOG_CRITICAL(...) lpcc_logprintf(LPCC_LOG_CRITICAL, __VA_ARGS__)

void lpcc_log_level(lpcc_log_level_t level);
void lpcc_logprintf(lpcc_log_level_t level, const char *fmt, ...);

#endif