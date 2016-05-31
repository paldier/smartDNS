#include "util_glb.h"
#include "statistics_glb.h"


/* 函数调用关系, 方便更美观的debug输出 */
CALL_RELATION g_call_relations[] = {
    {"get_query_domain", ""},
    {"process_mesg", "parse_dns,pass_acl,query_zone,sort_answer,cons_dns"},
    {"send_pkt", ""},
    {"pass_acl", ""},
    {"get_arr_index_by_type", ""},
    {"sort_answer", ""},
    {"query_zone", "get_rr_info"},
    {"add_dns_answer", ""},
    {"get_zone_info", ""},
    {"cons_dns_flag", ""},
    {"receive_pkt", ""},
    {"cons_dns", "cons_dns_flag,add_dns_answer"},
    {"parse_dns", "get_query_domain"},
    {"start_worker", "receive_pkt,process_mesg,send_pkt"},
    {"get_rr_info", ""},
    {NULL, NULL}
};

/* 注释信息, 方便输出阅读, 以enum_func_line为索引 */
STAT_INFO g_stat_infos[] = {
    {"dns.c", "mod_dns", "get_query_domain", "13"},
    {"dns.c", "mod_dns", "get_query_domain", "34"},
    {"worker.c", "mod_worker", "process_mesg", "23"},
    {"worker.c", "mod_worker", "process_mesg", "28"},
    {"worker.c", "mod_worker", "process_mesg", "38"},
    {"worker.c", "mod_worker", "process_mesg", "48"},
    {"worker.c", "mod_worker", "process_mesg", "54"},
    {"worker.c", "mod_worker", "process_mesg", "60"},
    {"engine.c", "mod_engine", "send_pkt", "150"},
    {"zone_query.c", "mod_zone", "pass_acl", "15"},
    {"zone.c", "mod_zone", "get_arr_index_by_type", "648"},
    {"zone.c", "mod_zone", "get_arr_index_by_type", "651"},
    {"sort.c", "mod_sort", "sort_answer", "15"},
    {"zone_query.c", "mod_zone", "query_zone", "24"},
    {"zone_query.c", "mod_zone", "query_zone", "40"},
    {"dns.c", "mod_dns", "add_dns_answer", "70"},
    {"zone.c", "mod_zone", "get_zone_info", "586"},
    {"zone.c", "mod_zone", "get_zone_info", "601"},
    {"dns.c", "mod_dns", "cons_dns_flag", "57"},
    {"engine.c", "mod_engine", "receive_pkt", "72"},
    {"engine.c", "mod_engine", "receive_pkt", "126"},
    {"dns.c", "mod_dns", "cons_dns", "165"},
    {"dns.c", "mod_dns", "cons_dns", "175"},
    {"dns.c", "mod_dns", "cons_dns", "183"},
    {"dns.c", "mod_dns", "parse_dns", "106"},
    {"dns.c", "mod_dns", "parse_dns", "120"},
    {"dns.c", "mod_dns", "parse_dns", "133"},
    {"dns.c", "mod_dns", "parse_dns", "143"},
    {"dns.c", "mod_dns", "parse_dns", "157"},
    {"worker.c", "mod_worker", "start_worker", "114"},
    {"worker.c", "mod_worker", "start_worker", "120"},
    {"zone.c", "mod_zone", "get_rr_info", "613"},
    {"zone.c", "mod_zone", "get_rr_info", "636"},
    {NULL, NULL, NULL, NULL}
};

