#include "util_glb.h"

GLB_VARS g_glb_vars;                /* 全局变量集合 */


GLB_VARS *get_glb_vars()
{
    return &g_glb_vars;
}

void *get_shared_mem()
{
    return g_glb_vars.sh_mem;
}







