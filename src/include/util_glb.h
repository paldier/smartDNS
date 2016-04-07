#ifndef UTIL_GLB_H
#define UTIL_GLB_H

#include <unistd.h>

/* 定义返回值 */
#define RET_OK      0
#define RET_ERR     -1

/* 定义进程角色 */
enum{
    PROCESS_ROLE_INVALID = 0,
    PROCESS_ROLE_MASTER,            /* 主进程 */
    PROCESS_ROLE_WORKER,            /* 查询处理进程 */
    PROCESS_ROLE_MONITOR,           /* 监控进程 */
    PROCESS_ROLE_SIGNALLER,         /* 信号处理 */
    PROCESS_ROLE_HELPER,            /* 打印帮助 */
    PROCESS_ROLE_TESTER,            /* 测试配置文件格式 */
    PROCESS_ROLE_MAX 
};

/* 聚集分散的全局变量 */
typedef struct st_glb_variables{
    int process_role;               /* 进程角色 */
    char *conf_file;                /* 配置文件, -f/-t指定 */
    char *signal;                   /* 处理信号, -s指定 */
}GLB_VARS;
extern GLB_VARS g_glb_vars;

#endif
