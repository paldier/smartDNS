#include "util_glb.h"
#include "engine_glb.h"
#include "sort_glb.h"
#include "sort.h"

int sort_answer(GLB_VARS *glb_vars, PKT *pkt)
{
    PKT_INFO *pkt_info = &pkt->info;

    if (pkt_info->rr_res_cnt == 0) {
        return RET_OK;
    }

    /* 优先级1(最高): 根据GeoIP排序 */

    /* 优先级2(次高): 根据负载排序 */

    return RET_OK;
}

