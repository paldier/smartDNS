#ifndef LOG_GLB_H
#define LOG_GLB_H

/**
 * 罗列宏操控示例, 储备!!!
 *
 * 示例一:
 *  #define STRING(x)  #x
 *  char *pChar = "hello";      ==>     char *pChar = STRING(hello);
 *
 * 示例二:
 *  #define makechar(x) #@x
 *  char ch = makechar(b);      ==>     char ch = 'b';
 *
 * 示例三:
 * #define link(n)  token##n
 * int link(9)  = 100;          ==>     int token9   = 100;
 *
 * <NOTE>嵌套宏遇到##或#不再展开的问题
 *  #define STRING(x)   #x
 *  char *pChar = STRING(__FILE__); ==> char *pChar = "__FILE__";
 *
 *  引入中间宏, 展开外层的宏
 *  #define _STRING(x)  #x
 *  #define STRING(x)   _STRING(x) 
 *  char *pChar = STRING(__FILE__); ==> char *pChar = "\"/usr/.../test.c\"";
 */

#include <syslog.h>

/**
 * 日志初始化
 * @param: void
 * @retval: RET_ERR/OK
 */
int log_init(void);

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
    log_base(__FILE__, __func__, __LINE__, LOG_ERR, fmt, ##__VA_ARGS__);\
}while(0);
#define SDNS_LOG_DEBUG(fmt, ...) do{\
    log_base(__FILE__, __func__, __LINE__, LOG_DEBUG, fmt, ##__VA_ARGS__);\
}while(0);
#define SDNS_LOG_WARN(fmt, ...) do{\
    log_base(__FILE__, __func__, __LINE__, LOG_WARNING, fmt, ##__VA_ARGS__);\
}while(0);

/** 
 * 定义统计模块儿需要的宏
 * 
 * 文件statistics.c由create_statistics_mod.py脚本自动生成
 *
 * 文件statistics_glb.h由create_statistics_mod.py脚本自动生成
 *
 * 由于stat_base()函数基于__LINE__宏计算统计索引值; 因此SDNS_STAT_X
 * 用于统计信息时, 不能跨行展开; 当参数过多, 不得不需要跨行展开时, 
 * 利用下属形式变更成单行:
 *      char tmp_stat_msg[STAT_MSG_LEN];
 *      snprintf(tmp_stat_msg, sizeof(tmp_stat_msg), ...)
 *      SDNS_LOG_STAT("%s", tmp_stat_msg);
 */
#include "statistics_glb.h"

#define CREATE_STATISTICS(mod, file_label)
#define STAT_FUNC_BEGIN
#define STAT_FUNC_END
#define STAT_MSG_LEN    256

/**
 * 日志输出标准函数
 * @param enum_id: [in], 统计宏索引, 参考~/include/statistics_glb.h
 * @param level: [in], 日志级别
 * @param fmt: [in], 格式字符串, 同printf
 * @retval: void
 */
void stat_base(int enum_id, int level, const char *fmt, ...) 
    __attribute__((format (printf, 3, 4)));

#define _TO_INDEX(arg1, arg2)  arg1##arg2
#define TO_INDEX(arg1, arg2)  _TO_INDEX(arg1, arg2)
#define SDNS_STAT_INFO(fmt, ...) do {\
    stat_base(TO_INDEX(STAT_FILE, __LINE__),\
            LOG_NOTICE, fmt, ##__VA_ARGS__);\
}while(0);
#define SDNS_STAT_TRACE() do {\
    stat_base(TO_INDEX(STAT_FILE, __LINE__),\
            LOG_NOTICE, "Entrance %s", __func__);\
}while(0);

#endif
