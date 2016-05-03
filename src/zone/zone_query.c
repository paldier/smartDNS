#include "util_glb.h"
#include "engine_glb.h"
#include "zone_glb.h"
#include "log_glb.h"
#include "zone_query.h"

/* 定义添加统计信息 */
#define     STAT_FILE      zone_query_c
CREATE_STATISTICS(mod_zone, zone_query_c)


STAT_FUNC_BEGIN ZONE * get_au_zone(GLB_VARS *glb_vars, char *domain)
{
    SDNS_STAT_TRACE();
    assert(glb_vars);
    assert(domain);

    char *au_domain = domain;
    ZONE *zone = NULL;

    while(au_domain) {
        zone = get_zone(glb_vars, au_domain);
        if (zone) {
            break;
        }

        au_domain = strchr(au_domain, '.');
        if (au_domain) {
            au_domain += 1;     /* 跳过'.' */
        }
    }

    return zone;
}STAT_FUNC_END

/***********************GLB FUNC*************************/

STAT_FUNC_BEGIN int pass_acl(GLB_VARS *glb_vars, PKT *pkt)
{
    SDNS_STAT_TRACE();
    assert(glb_vars);
    assert(pkt);

    /* 暂时空置 */
    return RET_OK;
}STAT_FUNC_END

STAT_FUNC_BEGIN int query_zone(GLB_VARS *glb_vars, PKT *pkt)
{
    SDNS_STAT_TRACE();
    assert(glb_vars);
    assert(pkt);

    char sub_domain[LABEL_LEN_MAX + 1];
    char *au_domain;
    PKT_INFO *pkt_info = &pkt->info;
    ZONE *zone;
    RR *rr;
    RR_DATA *rr_data;

    zone = get_au_zone(glb_vars, pkt_info->domain);
    if (zone == NULL) {
        SDNS_STAT_INFO("NULL au zone, [%s]", pkt_info->domain);
        return RET_ERR;
    }

    au_domain = strstr(pkt_info->domain, zone->name);
    snprintf(sub_domain, au_domain - pkt_info->domain, 
            "%s", pkt_info->domain);
    rr = get_rr(zone, sub_domain);
    if (rr == NULL) {
        SDNS_STAT_INFO("NULL RR, [sub-dom: %s]", sub_domain);
        return RET_ERR;
    }
    rr_data = &rr->data[get_arr_index_by_type(pkt_info->q_type)];

    pkt_info->rr_res_cnt = rr_data->cnt;
    pkt_info->rr_res_ttl = rr_data->ttl;
    SDNS_MEMCPY(pkt_info->rr_res, rr_data->data,
            sizeof(pkt_info->rr_res[0]) * pkt_info->rr_res_cnt);

    return RET_OK;
}STAT_FUNC_END

