#include "util_glb.h"
#include "statistics_glb.h"


/* 函数调用关系, 方便更美观的debug输出 */
CALL_RELATION g_call_relations[] = {
    {"get_query_domain", ""},
    {"process_mesg", "parse_dns,pass_acl,query_zone,sort_answer,cons_dns"},
    {"send_pkt", ""},
    {"get_arr_index_by_type", ""},
    {"get_zone_cfg", ""},
    {"pass_acl", ""},
    {"get_au_zone", "get_zone"},
    {"sort_answer", ""},
    {"query_zone", "get_au_zone,get_rr,get_arr_index_by_type"},
    {"add_dns_answer", ""},
    {"get_rr", ""},
    {"get_zone", ""},
    {"cons_dns_flag", ""},
    {"receive_pkt", ""},
    {"cons_dns", "cons_dns_flag,add_dns_answer"},
    {"parse_dns", "get_query_domain"},
    {"start_worker", "receive_pkt,process_mesg,send_pkt"},
    {NULL, NULL}
};

/* 注释信息, 方便输出阅读, 以enum_func_line为索引 */
STAT_INFO g_stat_infos[] = {
    {"dns.c", "mod_dns", "get_query_domain", "13"},
    {"dns.c", "mod_dns", "get_query_domain", "34"},
    {"worker.c", "mod_worker", "process_mesg", "24"},
    {"worker.c", "mod_worker", "process_mesg", "29"},
    {"worker.c", "mod_worker", "process_mesg", "39"},
    {"worker.c", "mod_worker", "process_mesg", "49"},
    {"worker.c", "mod_worker", "process_mesg", "55"},
    {"worker.c", "mod_worker", "process_mesg", "61"},
    {"engine.c", "mod_engine", "send_pkt", "150"},
    {"zone.c", "mod_zone", "get_arr_index_by_type", "601"},
    {"zone.c", "mod_zone", "get_arr_index_by_type", "604"},
    {"zone.c", "mod_zone", "get_zone_cfg", "538"},
    {"zone.c", "mod_zone", "get_zone_cfg", "542"},
    {"zone.c", "mod_zone", "get_zone_cfg", "552"},
    {"zone_query.c", "mod_zone", "pass_acl", "40"},
    {"zone_query.c", "mod_zone", "get_au_zone", "15"},
    {"sort.c", "mod_sort", "sort_answer", "15"},
    {"zone_query.c", "mod_zone", "query_zone", "49"},
    {"zone_query.c", "mod_zone", "query_zone", "61"},
    {"zone_query.c", "mod_zone", "query_zone", "70"},
    {"dns.c", "mod_dns", "add_dns_answer", "70"},
    {"zone.c", "mod_zone", "get_rr", "575"},
    {"zone.c", "mod_zone", "get_rr", "581"},
    {"zone.c", "mod_zone", "get_rr", "595"},
    {"zone.c", "mod_zone", "get_zone", "558"},
    {"zone.c", "mod_zone", "get_zone", "568"},
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
    {"worker.c", "mod_worker", "start_worker", "118"},
    {"worker.c", "mod_worker", "start_worker", "124"},
    {NULL, NULL, NULL, NULL}
};

