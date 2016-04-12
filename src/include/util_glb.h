#ifndef UTIL_GLB_H
#define UTIL_GLB_H

#include <unistd.h>

/* 定义返回值 */
#define RET_OK      0
#define RET_ERR     -1

/* 定义进程角色 */
enum{
    PROCESS_ROLE_INVALID = 0,
    PROCESS_ROLE_MASTER = 1<<0,     /* 主进程 */
    PROCESS_ROLE_WORKER = 1<<1,     /* 查询处理进程 */
    PROCESS_ROLE_MONITOR = 1<<2,    /* 监控进程 */
    PROCESS_ROLE_SIGNALLER = 1<<3,  /* 信号处理 */
    PROCESS_ROLE_HELPER = 1<<4,     /* 打印帮助 */
    PROCESS_ROLE_TESTER = 1<<5,     /* 测试配置文件格式 */
    PROCESS_ROLE_MAX = 1<<15
};

/* 聚集分散的全局变量 */
typedef struct st_glb_variables{
    int process_role;               /* 进程角色 */
    char *conf_file;                /* 配置文件, -f指定 */
    char *signal;                   /* 处理信号, -s指定 */
    void *conf;                     /* 配置信息在内存中的组织形式 */
    void *zone;                     /* 域信息 */
    void *sh_mem;                   /* 共享内存 */
}GLB_VARS;

#endif
