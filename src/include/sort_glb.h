#ifndef SORT_GLB_H
#define SORT_GLB_H

/**
 * 根据配置的策略, 排序查询结果
 * @param glb_vars: [in], 全局信息集合
 * @param pkt: [in][out], 报文信息结构
 * @retval: RET_OK/RET_ERR
 *
 * @NOTE
 *  1) 此函数会修正重排PKT->info.rr_res[]
 */
int sort_answer(GLB_VARS *glb_vars, PKT *pkt);

#endif
