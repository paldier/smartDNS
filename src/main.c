/**
 * 智能DNS入口文件, 协调各模块儿运行
 */
#include "util_glb.h"
#include "engine_glb.h"
#include "worker_glb.h"
#include "signal_glb.h"
#include "zone_glb.h"
#include "option_glb.h"
#include "monitor_glb.h"
#include "log_glb.h"


int main(int argc, char **argv)
{
    INIT_GLB_VARS();
    SET_PROCESS_ROLE(PROCESS_ROLE_MASTER);
    SET_CHILD_PROCESS(PROCESS_ROLE_MASTER, getpid());

    /* 此处注册清理函数, 以便主进程由于某些原因退出时, kill掉
     * 启动的所有子进程. 但man atexit可知, 通过fork的子进程会
     * 继承atexit的注册链, 而exec后将抛弃此注册链.
     * <NOTE> 此处不考虑fork子进程也执行此函数带来的影响 */
    (void)atexit(kill_child_all);

    if (log_init() == RET_ERR) {
        SDNS_LOG_ERR("LOG init failed");
        exit(EXIT_FAILURE);
    }

    if (zone_init() == RET_ERR) {
        SDNS_LOG_ERR("ZONE init failed");
        exit(EXIT_FAILURE);
    }

    if (get_options(argc, argv) == RET_ERR) {
        SDNS_LOG_ERR("parse cmdline failed");
        usage_help();
        exit(EXIT_FAILURE);
    }

    if (IS_PROCESS_ROLE(PROCESS_ROLE_SIGNALLER)) {
        process_option_signal();
        exit(EXIT_SUCCESS);
    }

    if (IS_PROCESS_ROLE(PROCESS_ROLE_HELPER)) {
        usage_help();
        exit(EXIT_SUCCESS);
    }

    if (start_monitor() == RET_ERR) {
        exit(EXIT_FAILURE);
    }

    if (parse_conf() == RET_ERR) {
        exit(EXIT_FAILURE);
    }
    
    if (IS_PROCESS_ROLE(PROCESS_ROLE_TESTER)) {
        SDNS_LOG_DEBUG("配置文件测试OK");
        print_parse_res();
        exit(EXIT_SUCCESS);
    }

    if (pkt_engine_init() == RET_ERR) {
        SDNS_LOG_ERR("engine init failed");
        exit(EXIT_FAILURE);
    }

    if (set_required_signal() == RET_ERR
            || block_required_signal() == RET_ERR) {
        SDNS_LOG_ERR("signal init failed");
        exit(EXIT_FAILURE);
    }

    start_worker();

    if (start_pkt_engine() == RET_ERR) {
        SDNS_LOG_ERR("start engine failed");
        exit(EXIT_FAILURE);
    }

    for(;;) {
        (void)wait_required_signal();
        (void)process_signals();
    }

    exit(EXIT_SUCCESS);
}




