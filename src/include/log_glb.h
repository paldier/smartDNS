#ifndef LOG_GLB_H
#define LOG_GLB_H

#include <syslog.h>
#include "util_glb.h"

/**
 * 日志初始化
 * @param: void
 * @retval: RET_ERR/OK
 */
int log_init();

/**
 * 日志输出标准函数
 * @param file: [in], 输出日志时代码所在的文件
 * @param func: [in], 输出日志时代码所在的函数
 * @param line: [in], 输出日志时代码所在的行
 * @param level: [in], 日志级别
 * @param fmt: [in], 格式字符串, 同printf
 * @retval: void
 */
void log_base(const char *file, const char *func, int line, 
        int level, const char *fmt, ...) 
    __attribute__((format (printf, 5, 6)));

/**
 * 日志输出宏
 */
#define SDNS_LOG_ERR(fmt, ...) do{\
    log_base(__FILE__, __func__, __LINE__, LOG_ERR,\
            fmt, ##__VA_ARGS__);\
}while(0);
#define SDNS_LOG_DEBUG(fmt, ...) do{\
    log_base(__FILE__, __func__, __LINE__, LOG_DEBUG,\
            fmt, ##__VA_ARGS__);\
}while(0);
#define SDNS_LOG_WARN(fmt, ...) do{\
    log_base(__FILE__, __func__, __LINE__, LOG_WARNING,\
            fmt, ##__VA_ARGS__);\
}while(0);

#endif
