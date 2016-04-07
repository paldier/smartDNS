#include <stdio.h>      /* for vsnprintf() */
#include <stdarg.h>     /* for va_list */
#include <unistd.h>     /* for write() */
#include <string.h>     /* for strlen() */
#include "log.h"


void log_base(const char *file, const char *func, int line, int fd, 
        const char *fmt, ...)
{
    char tmp_str[MAX_LOG_STR_LEN] = {0};
    va_list   args;

    va_start(args, fmt);
    vsnprintf(tmp_str, MAX_LOG_STR_LEN - 1, fmt, args);
    va_end(args);

    if (fd) {
        write(fd, (const void *)tmp_str, strlen(tmp_str));
        write(fd, "\n", strlen("\n"));
    } else {
        //syslog();
    }
}



