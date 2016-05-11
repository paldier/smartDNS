#ifndef ZONE_QUERY_H
#define ZONE_QUERY_H

#include "zone.h"

/**
 * 获取查询域名对应的权威域配置信息
 * @param q_domain: [in], 查询的域名
 * @retval: NULL/ZONE *, 对应的权威域
 */
ZONE *get_au_zone(char *q_domain);

#endif
