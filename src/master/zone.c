#include "cfg_glb.h"
#include "zone.h"

/* 存储域配置文件*.zone的元数据及处理指针 */
#if 0
static CFG_TYPE s_cfg_type_arr[] = {
    {"$TTL", },
    {"$ORIGIN", },
    {"IN", },
    {"A", },
    {"*", },
    {"", (token_handler)NULL},
};
#endif

/**
 * RR记录类型中的TYPE描述
 */
enum {
    TYPE_A = 1,
    TYPE_ANY = 255,
};
static const struct {
    const char *name;       /* 配置文件中的名字 */
    int value;              /* 报文中的值 */
} types[] = {
    {"A",   TYPE_A},
    {0, 0}
};

int zone_parse(GLB_VARS *glb_vars)
{
    return RET_ERR;
}


