#include <stdarg.h>     /* for va_list */
#include "util_glb.h"
#include "log_glb.h"
#include "log.h"

/* 单条日志的最大长度 */
#define MAX_LOG_STR_LEN     1024

/* 自定义日志文件描述符 */
static FILE *s_log_fd;

/* 日志级别 */
static struct st_log_level {
    char name[32];
} s_log_level[] = {
    {"LOG_EMERG"},
    {"LOG_ALERT"},
    {"LOG_CRIT"},
    {"LOG_ERR"},
    {"LOG_WARNING"},
    {"LOG_NOTICE"},
    {"LOG_INFO"},
    {"LOG_DEBUG"},
    {""}
};

/* 统计信息汇总结构 */
static uint64_t s_stat_res[enum_file_label_line_max] = {0};
static bool s_trace_pkt = true;

/***********************GLB FUNC*************************/

int log_init()
{
#ifdef DNS_DEBUG
    openlog("smartDNS", LOG_NDELAY|LOG_PID|LOG_PERROR, LOG_LOCAL5);
#else
    openlog("smartDNS", LOG_NDELAY|LOG_PID, LOG_LOCAL5);
#endif
    
    SDNS_LOG_DEBUG("log init OK!");
    return RET_OK;
}

void log_base(const char *file, const char *func, int line, 
        int level, const char *fmt, ...)
{
    char tmp_str[MAX_LOG_STR_LEN] = {0};
    va_list   args;

    va_start(args, fmt);
    vsnprintf(tmp_str, MAX_LOG_STR_LEN, fmt, args);
    va_end(args);

    if (s_log_fd) {
        fprintf(s_log_fd, "<%s><%s|%s|%d>: %s\n", s_log_level[level].name,
                file, func, line, tmp_str);
    } else {
        if (level < LOG_DEBUG) {
            syslog(level, "<%s><%s|%s|%d>: %s", s_log_level[level].name,
                    file, func, line, tmp_str);
        } else {
            syslog(level, "%s", tmp_str);
        }
    }
}

void stat_base(int enum_id, int level, const char *fmt, ...) 
{
    assert(enum_id < enum_file_label_line_max);

    /* 统计: 必要步骤 */
    s_stat_res[enum_id]++;

    /* 报文示踪: 通过全局开关控制 */
    if (s_trace_pkt) {
        char tmp_str[MAX_LOG_STR_LEN] = {0};
        va_list   args;

        /* 仅考虑worker进程 */
        if (!IS_PROCESS_ROLE(PROCESS_ROLE_WORKER)) {
            return;
        }

        va_start(args, fmt);
        vsnprintf(tmp_str, MAX_LOG_STR_LEN, fmt, args);
        va_end(args);

        log_base(g_stat_infos[enum_id].f_name, 
                g_stat_infos[enum_id].func, 
                atoi(g_stat_infos[enum_id].line), 
                level, "%s", tmp_str);
    }
}


