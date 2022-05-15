#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>

#include "lpcc_log.h"

static lpcc_log_level_t s_currentlevel = LPCC_LOG_INFO;

void lpcc_log_level(lpcc_log_level_t level) {
    s_currentlevel = level;
}

void lpcc_logprintf(lpcc_log_level_t level, const char *fmt, ...) {

    if(level < s_currentlevel) return;

    uint8_t color_code = 0U;
    switch(level) {
        case LPCC_LOG_DEBUG:
        color_code = 96U;
        break;
        case LPCC_LOG_INFO:
        color_code = 92U;
        break;
        case LPCC_LOG_WARN:
        color_code = 93U;
        break;
        case LPCC_LOG_ERROR:
        color_code = 91U;
        break;
        case LPCC_LOG_CRITICAL:
        color_code = 41U;
        break;

        default:
        break;
    }

    fprintf(stderr, "\033[%dm", color_code);

    va_list ap;
    va_start(ap, fmt);

    vfprintf(stderr, fmt, ap);

    va_end(ap);

    fprintf(stderr, "\033[m\r\n");
}