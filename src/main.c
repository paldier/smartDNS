/**
 * 智能DNS入口文件, 协调各模块儿运行
 */
#include "util_glb.h"
#include "cfg_glb.h"
#include "engine_glb.h"
#include "worker_glb.h"
#include "signal_glb.h"
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
        SDNS_LOG_ERR("log init failed");
        return 1;
    }

    /* 处理命令行参数 */
    if (get_options(argc, argv, &g_glb_vars) == RET_ERR) {
        usage_help();
        SDNS_LOG_ERR("parse cmd line failed");
        return 1;
    }

    /* 输出帮助信息并退出, <NOTE>帮助信息和其他参数互斥 */
    if (g_glb_vars.process_role & PROCESS_ROLE_HELPER) {
        usage_help();
        return 0;
    }

    /* 解析配置文件.conf */
    if (cfg_parse(&g_glb_vars) == RET_ERR) {
        SDNS_LOG_ERR("parse .conf failed");
        return 1;
    }

    /* 解析域信息文件*.zone */
    if (zone_parse(&g_glb_vars) == RET_ERR) {
        SDNS_LOG_ERR("parse .zone failed");
        return 1;
    }

    /* 报文收发引擎初始化 */
    if (pkt_engine_init() == RET_ERR) {
        SDNS_LOG_ERR("engine init failed");
        return 1;
    }

    /* 处理信号 */
    if (set_required_signal() == RET_ERR
            || block_required_signal() == RET_ERR) {
        SDNS_LOG_ERR("signal init failed");
        return 1;
    }

    /* 启动工作进程 */
    start_worker(&g_glb_vars);

    /* 启动收发报文引擎 */
    if (start_pkt_engine() == RET_ERR) {
        SDNS_LOG_ERR("start engine failed");
        return 1;
    }

    /* 主循环 */
    for(;;) {
        /* 等待信号 */
        (void)wait_required_signal();

        /* 处理信号 */
        (void)process_signals(&g_glb_vars);
    }

    /* 释放动态申请的资源 */
    release_resource(&g_glb_vars);

    return 0;
}




