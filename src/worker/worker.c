#include <sys/mman.h>       /* for mprotect() */
#include "util_glb.h"
#include "engine_glb.h"
#include "worker_glb.h"
#include "signal_glb.h"
#include "dns_glb.h"
#include "zone_glb.h"
#include "sort_glb.h"
#include "mem_glb.h"
#include "log_glb.h"
#include "worker.h"

/* 定义添加统计信息 */
#define     STAT_FILE      worker_c
CREATE_STATISTICS(mod_worker, worker_c)

void start_other_worker()
{
    return;
}

STAT_FUNC_BEGIN int process_mesg(PKT *pkt)
{
    SDNS_STAT_TRACE();
    assert(pkt);

    /* 解析DNS报文 */
    if (parse_dns(pkt) == RET_ERR) {
        SDNS_STAT_INFO("parse failed");
        return RET_ERR;
    }

    /* 匹配ACL规则 */
    if (pass_acl(pkt) == RET_ERR) {
        char tmp_addr[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(pkt->info.src_ip.ip4),
                tmp_addr, INET_ADDRSTRLEN);
        SDNS_STAT_INFO("NOT pass ACL, [%s]", tmp_addr); 
        return RET_ERR;
    }

    /* 查询RR记录 */
    if (query_zone(pkt) == RET_ERR) {
        char tmp_stat_msg[STAT_MSG_LEN];
        snprintf(tmp_stat_msg, sizeof(tmp_stat_msg), 
                "query zone failed, [domain: %s]/[type: %d]/[class: %d]",
                pkt->info.domain, pkt->info.q_type, pkt->info.q_class);
        SDNS_STAT_INFO("%s", tmp_stat_msg);
        return RET_ERR;
    }

    /* 排序算法: DRF + GeoIP */
    if (sort_answer(pkt) == RET_ERR) {
        SDNS_STAT_INFO("sort failed");
        return RET_ERR;
    }

    /* 组装应答报文 */
    if (cons_dns(pkt) == RET_ERR) {
        SDNS_STAT_INFO("cons failed");
        return RET_ERR;
    }

    return RET_OK;
}STAT_FUNC_END

/***********************GLB FUNC*************************/

STAT_FUNC_BEGIN void start_worker()
{
    /**
     * 多进程同时监听某个UDP插口, 需要在一个进程建立socket, 然后通过
     * fork()的方式建立其他进程, 这样直接继承建立的socket; 当然多个
     * 进程通过阻塞读方式会有"惊群"现象.
     *
     * 如果在多个进程分别调用socket/bind()建立插口, 则只有第一个进程
     * 能接收到报文, 其他进程不响应!!!
     *
     * 如果性能问题, 后续采用epoll事件分发模型, 主worker收, 其他worker
     * 处理!
     */
    pid_t child_pid = 0;

    /* 启动第一个worker进程 */
    child_pid = fork();
    if (child_pid) {
        SDNS_LOG_DEBUG("In parent, fork worker[%d]", child_pid);
        return;
    }
    SET_PROCESS_ROLE(PROCESS_ROLE_WORKER);
    if (mprotect(get_shared_mem(), get_sh_mem_total_size(), 
                PROT_READ) == -1) {
        SDNS_LOG_ERR("mprotect err, [%s]\n", strerror(errno));
    }

    /* 引擎第二阶段初始化 */
    if (pkt_engine_init_2() == RET_ERR) {
        SDNS_LOG_ERR("In worker[%d], engine init failed", getpid());
        return;
    }

    /* 启动其他worker进程 */
    start_other_worker();

    /* 去除信号屏蔽 */
    if (clear_mask_signal() == RET_ERR) {
        SDNS_LOG_ERR("In worker[%d], clear signal mask failed", getpid());
        return;
    }

    /* 主处理流程 */
    for (;;) {
        PKT *pkt = NULL;
    
        /* 接收报文 */
        if (receive_pkt(&pkt) == RET_ERR) {
            SDNS_STAT_INFO("receive pkt error"); 
            continue;
        }

        /* 处理数据 */
        if (process_mesg(pkt) == RET_ERR) {
            SDNS_STAT_INFO("process pkt error"); 
            continue;
        }

        /* 忽略发送失败 */
        (void)send_pkt(pkt);
    }
}STAT_FUNC_END
