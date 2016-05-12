#include "util_glb.h"
#include "zone_glb.h"
#include "mem_glb.h"
#include "log_glb.h"
#include "check_main.h"

int shared_mem_init()
{
    int tmp_ret;

    INIT_GLB_VARS();
    SET_PROCESS_ROLE(PROCESS_ROLE_MASTER);
    snprintf(get_glb_vars()->conf_file, sizeof(get_glb_vars()->conf_file),
            "%s", "../../conf/master.conf");

    tmp_ret = log_init();
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = mem_init();
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = zone_init();
    ck_assert_int_eq(tmp_ret, RET_OK);

    tmp_ret = create_shared_mem_for_test();
    ck_assert_int_eq(tmp_ret, RET_OK);

    return tmp_ret;
}


