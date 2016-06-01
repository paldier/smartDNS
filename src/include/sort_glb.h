#ifndef SORT_GLB_H
#define SORT_GLB_H

/**
 * 排序所需要的资源初始化, 如GeoIP2库
 */
int sort_init(void);

/**
 * 根据配置的策略, 排序查询结果
 * @param pkt: [in][out], 报文信息结构
 * @retval: RET_OK/RET_ERR
 *
 * @NOTE
 *  1) 此函数会修正重排PKT->info.rr_res[]
 */
int sort_answer(PKT *pkt);

#endif
