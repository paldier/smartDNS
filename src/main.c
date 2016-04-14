/**
 * 智能DNS入口文件, 协调各模块儿运行
 */
#include <string.h>         /* for memset() */
#include "util_glb.h"
#include "cfg_glb.h"
#include "log_glb.h"

GLB_VARS g_glb_vars;

void release_resource(GLB_VARS *glb_vars)
{
    /* 释放.conf配置信息 */
    release_conf(glb_vars);

    /* 释放.zone配置信息 */
    release_zone(glb_vars);

    /* 释放其他资源 */
}

int main(int argc, char **argv)
{
    /* 初始化全局变量 */
    (void)memset((void *)&g_glb_vars, 0, sizeof(GLB_VARS));

    /* 日志初始化 */
    if (log_init(&g_glb_vars) == RET_ERR) {
        return 1;
    }

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

    /* 解析配置文件.conf */
    if (cfg_parse(&g_glb_vars) == RET_ERR) {
        return 1;
    }

    /* 解析域信息文件*.zone */
    if (zone_parse(&g_glb_vars) == RET_ERR) {
        return 1;
    }

#if 0
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

    /* 释放动态申请的资源 */
    release_resource(&g_glb_vars);

    return 0;
}




