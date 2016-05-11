#include "util_glb.h"
#include "engine_glb.h"
#include "sort_glb.h"
#include "log_glb.h"
#include "sort.h"

/* 定义添加统计信息 */
#define     STAT_FILE      sort_c
CREATE_STATISTICS(mod_sort, sort_c)

/***********************GLB FUNC*************************/

STAT_FUNC_BEGIN int sort_answer(PKT *pkt)
{
    SDNS_STAT_TRACE();
    assert(pkt);
    assert(pkt->info.rr_res_cnt);

    /* 优先级1(最高): 根据GeoIP排序 */

    /* 优先级2(次高): 根据负载排序 */

    return RET_OK;
}STAT_FUNC_END

