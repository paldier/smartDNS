#include "util_glb.h"
#include "engine_glb.h"
#include "zone_glb.h"
#include "log_glb.h"
#include "zone.h"
#include "zone_query.h"

/* 定义添加统计信息 */
#define     STAT_FILE      zone_query_c
CREATE_STATISTICS(mod_zone, zone_query_c)


STAT_FUNC_BEGIN int pass_acl(void *pkt)
{
    SDNS_STAT_TRACE();
    assert(pkt);

    /* 暂时空置 */
    return RET_OK;
}STAT_FUNC_END

STAT_FUNC_BEGIN int query_zone(void *pkt)
{
    SDNS_STAT_TRACE();
    assert(pkt);

    PKT_INFO *pkt_info = &((PKT *)pkt)->info;
    RR_INFO rr_info[RR_PER_TYPE_MAX];
    int res_num = RR_PER_TYPE_MAX;
    int tmp_ret;

    rr_info[0].type = pkt_info->q_type;
    rr_info[0].rr_class = pkt_info->q_class;
    tmp_ret = get_rr_info(pkt_info->domain, rr_info, &res_num);
    if (tmp_ret == RET_ERR || res_num == 0) {
        char tmp_stat_msg[STAT_MSG_LEN];
        snprintf(tmp_stat_msg, sizeof(tmp_stat_msg),
                "NOT found rr info, [domain: %s]/[type: %d]/[class: %d]",
                pkt_info->domain, pkt_info->q_type, pkt_info->q_class);
        SDNS_STAT_INFO("%s", tmp_stat_msg);
        return RET_ERR;
    }
    
    pkt_info->rr_res_cnt = res_num;
    for (int i=0; i<res_num; i++) {
        pkt_info->rr_res_ttl[i] = rr_info[i].ttl;
        SDNS_MEMCPY(&pkt_info->rr_res[i], &rr_info[i].data,
                sizeof(pkt_info->rr_res[i]));
    }

    return RET_OK;
}STAT_FUNC_END

