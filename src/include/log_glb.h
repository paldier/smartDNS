#ifndef LOG_GLB_H
#define LOG_GLB_H

#include "util_glb.h"

/**
 * 日志输出标准函数
 * @param err_num: [in], 错误码, errno或0(自定义错误)
 * @param fd: [in], 日志输出文件描述符
 * @param fmt: [in], 格式字符串, 同printf
 * @retval: void
 */
void log_base(const char *file, const char *func, int line, int fd, 
        const char *fmt, ...) __attribute__((format (printf, 5, 6)));

/**
 * 输出到标准错误输出
 */
#define LOG_ERR(fmt, ...) do{\
    log_base(__FILE__, __func__, __LINE__, STDERR_FILENO, fmt, ##__VA_ARGS__);\
}while(0);

#endif
