/**
 * 智能DNS入口文件, 协调各模块儿运行
 */
#include <string.h>         /* for memset() */
#include "util_glb.h"
#include "cfg_glb.h"
#include "log_glb.h"

GLB_VARS g_glb_vars;

int main(int argc, char **argv)
{
    /* 初始化全局变量 */
    (void)memset((void *)&g_glb_vars, 0, sizeof(GLB_VARS));

    /* 处理命令行参数 */
    if (get_options(argc, argv, &g_glb_vars) == RET_ERR) {
        usage_help();
        return 1;
    }

    /* 输出帮助信息并退出, <NOTE>帮助信息和其他参数互斥 */
    if (g_glb_vars.process_role & PROCESS_ROLE_HELPER) {
        usage_help();
        return 0;
    }

    /* 解析配置文件 */
    if (cfg_parse(&g_glb_vars) == RET_ERR) {
        return 1;
    }

#if 0
    /* 日志初始化 */
    log_init();

    /* 报文收发引擎初始化 */
    pkt_engine_init();

    /* 启动工作进程 */
    start_worker();

    /* 启动收发报文引擎 */
    start_pkt_engine();

    /* 主循环, 处理信号 */
    set_signal_handlers();
    for(;;) {
        /* SIG_CLD */
    }
#endif

    return 0;
}












