#include <libgen.h>         /* for dirname() */
#include "util_glb.h"
#include "engine_glb.h"
#include "sort_glb.h"
#include "log_glb.h"
#include "maxminddb.h"
#include "sort.h"

/* 定义添加统计信息 */
#define     STAT_FILE      sort_c
CREATE_STATISTICS(mod_sort, sort_c)

#define GEOIP2_DB_NAME      "GeoLite2-City.mmdb"

int sort_init()
{
    char db_name[255];      /* magic 255: 文件绝对路径名长度上限 */
    int tmp_ret;

    tmp_ret = readlink("/proc/self/exe", db_name, sizeof(db_name));
    if (tmp_ret == -1) {
        SDNS_LOG_ERR("get exe path failed!!! [%s]", strerror(errno));
        return RET_ERR;
    }
    db_name[sizeof(db_name) - 1] = 0;
    tmp_ret = strlen(dirname(db_name));
    tmp_ret = snprintf(db_name + tmp_ret, 
            sizeof(db_name) - tmp_ret, "/%s", GEOIP2_DB_NAME); 

    get_glb_vars()->mmdb = SDNS_MALLOC(sizeof(MMDB_s));

    tmp_ret = MMDB_open(db_name, MMDB_MODE_MMAP,
            (MMDB_s*)(get_glb_vars()->mmdb));
    if (MMDB_SUCCESS != tmp_ret) {
        SDNS_LOG_ERR("Can't open %s - %s", db_name, MMDB_strerror(tmp_ret));
        if (MMDB_IO_ERROR == tmp_ret) {
            SDNS_LOG_ERR("IO error: %s", strerror(errno));
        }

        return RET_ERR;
    }

    /* <NOTE!!!>仅仅打开数据库, 但不关闭, 希望程序退出时自动释放资源 */
    //MMDB_close(&mmdb);
    return RET_OK;
}

STAT_FUNC_BEGIN int sort_answer(PKT *pkt)
{
    SDNS_STAT_TRACE();

    MMDB_s *mmdb;
    struct sockaddr_in addr;
    int tmp_ret;

    assert(pkt);
    assert(pkt->info.rr_res_cnt);

    /* 优先级1(最高): 根据GeoIP排序 */
    addr.sin_family = AF_INET;      /* 目前只考虑IPv4 */
    addr.sin_addr.s_addr = pkt->info.src_ip.ip4;
    mmdb = get_glb_vars()->mmdb;

    MMDB_lookup_result_s result = 
        MMDB_lookup_sockaddr(mmdb, (struct sockaddr *)&addr, &tmp_ret);
    if (tmp_ret != MMDB_SUCCESS) {
        SDNS_STAT_INFO("libmaxminddb failed[%s]",MMDB_strerror(tmp_ret));
        return RET_OK;              /* 排序函数不能导致查询中断 */
    }

    MMDB_entry_data_list_s *entry_data_list = NULL;
    if (result.found_entry) {
        tmp_ret = MMDB_get_entry_data_list(&result.entry, &entry_data_list);
        if (tmp_ret != MMDB_SUCCESS) {
            SDNS_STAT_INFO("lookup entry err[%s]", MMDB_strerror(tmp_ret));
        } else {
#if 1
            if (entry_data_list != NULL) {
                MMDB_dump_entry_data_list(stdout, entry_data_list, 2);
            }
#else
            MMDB_entry_data_s   entry_data;
            /* 根据json结构选择具体的项<NOTE!!!>不能忽略末尾的NULL */
            status = MMDB_get_value(&result.entry, &entry_data, 
                    "city", "names", "en", NULL);
            if (status == MMDB_SUCCESS && entry_data.has_data) {
                char tmp_str[255];      /* magic 255: 城市名最大长度 */
                /* <NOTE>此时应该按照已知的类型做相应的处理 */
                snprintf(tmp_str, entry_data.data_size, "%s", entry_data.utf8_string);
                printf("city name: %s\n", tmp_str);
            }
#endif
        }
    } else {
        char tmp_addr[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, &(pkt->info.src_ip.ip4),
                tmp_addr, INET_ADDRSTRLEN);
        SDNS_STAT_INFO("No entry for IP[%s]", tmp_addr);
    }
    MMDB_free_entry_data_list(entry_data_list);

    /* 优先级2(次高): 根据负载排序 */

    return RET_OK;
}STAT_FUNC_END

