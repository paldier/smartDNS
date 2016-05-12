/**
 * 智能DNS入口文件, 协调各模块儿运行
 */
#include "util_glb.h"
#include "engine_glb.h"
#include "worker_glb.h"
#include "signal_glb.h"
#include "zone_glb.h"
#include "mem_glb.h"
#include "option_glb.h"
#include "log_glb.h"


int main(int argc, char **argv)
{
    INIT_GLB_VARS();
    SET_PROCESS_ROLE(PROCESS_ROLE_MASTER);

    if (log_init() == RET_ERR) {
        SDNS_LOG_ERR("LOG init failed");
        return RET_ERR;
    }

    if (mem_init() == RET_ERR) {
        SDNS_LOG_ERR("MEM init failed");
        return RET_ERR;
    }

    if (zone_init() == RET_ERR) {
        SDNS_LOG_ERR("ZONE init failed");
        return RET_ERR;
    }

    if (get_options(argc, argv) == RET_ERR) {
        SDNS_LOG_ERR("parse cmdline failed");
        usage_help();
        return 1;
    }

    if (IS_PROCESS_ROLE(PROCESS_ROLE_HELPER)) {
        usage_help();
        return 0;
    }

    if (parse_conf_for_test() == RET_ERR) {
        return 1;
    }
    
    if (IS_PROCESS_ROLE(PROCESS_ROLE_TESTER)) {
        SDNS_LOG_DEBUG("配置文件测试OK");

        print_parse_res();
        return 0;
    }

    if (create_shared_mem() == RET_ERR) {
        return 1;
    }

    /*****************以下代码应该跑在monitor进程***************/
    SET_PROCESS_ROLE(PROCESS_ROLE_MONITOR);
    if (parse_conf() == RET_ERR) {
        return 1;
    }
    /*****************以上代码应该跑在monitor进程***************/

    if (pkt_engine_init() == RET_ERR) {
        SDNS_LOG_ERR("engine init failed");
        return 1;
    }

    if (set_required_signal() == RET_ERR
            || block_required_signal() == RET_ERR) {
        SDNS_LOG_ERR("signal init failed");
        return 1;
    }

    start_worker(get_glb_vars());

    if (start_pkt_engine() == RET_ERR) {
        SDNS_LOG_ERR("start engine failed");
        return 1;
    }

    for(;;) {
        (void)wait_required_signal();
        (void)process_signals(get_glb_vars());
    }

    unlink_shared_mem();

    return 0;
}




