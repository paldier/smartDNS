#ifndef UTIL_GLB_H
#define UTIL_GLB_H

#include <stdlib.h>         /* for malloc/free/realloc()*/
#include <string.h>         /* for memset/memcpy/strerror() */
#include <sys/types.h>
#include <unistd.h>         /* for fork/getpid()/pid_t */
#include <errno.h>          /* for errno */
#include <stdio.h>          /* for fopen/printf() */
#include <stdint.h>         /* for uint32_t/.. */ 
#include <arpa/inet.h>      /* for htons/inet_pton()/.. */
#include <stdbool.h>        /* for true/false */
#ifndef DNS_DEBUG
#define NDEBUG
#endif
#include <assert.h>         /* for assert() */

/* 定义返回值 */
#define RET_OK      0
#define RET_ERR     -1


#define DNS_PORT        htons(53)   /* 定义DNS知名端口号 */
#define LABEL_LEN_MAX   63      /* 域名中单个label长度最大值 */
#define DOMAIN_LEN_MAX  255     /* 域名最大长度, 包括结尾的0x00 label */
#define RR_TYPE_MAX     1       /* 支持的域名种类 */
#define RR_PER_TYPE_MAX 1       /* 单个域名同类型支持的记录数 */

/* 定义进程角色 */
enum{
    PROCESS_ROLE_INVALID = 0,
    PROCESS_ROLE_MASTER = 1<<0,     /* 主进程 */
    PROCESS_ROLE_WORKER = 1<<1,     /* 查询处理进程 */
    PROCESS_ROLE_MONITOR = 1<<2,    /* 监控进程 */
    PROCESS_ROLE_SIGNALLER = 1<<3,  /* 信号处理 */
    PROCESS_ROLE_HELPER = 1<<4,     /* 打印帮助 */
    PROCESS_ROLE_TESTER = 1<<5,     /* 测试配置文件格式 */
    PROCESS_ROLE_MAX = 1<<15
};

/**
 * 聚集分散的全局变量 
 * <NOTE>
 *  1) 使用void *是为了避免过多的头文件引用
 *  2) conf和zones分别代表master.conf和xxx.zone文件的解析结果
 *      conf包含了域的控制信息, 如ACL/域信息文件等
 *      zones则包含了具体的域记录, 如RR/SOA等
 *  3) 域名请求时, 搜索conf/zones得到对应的RR记录, 并将结果写
 *      入高速缓存; 后续直接走高速缓存(各worker进程独有)
 *  4) 后续conf和zones移动到共享内存
 */
typedef struct st_glb_variables{
#define CONF_FILE_LEN   32
#define SIGNAL_STR_LEN  32
#define WORKER_NUM_MAX  5
    int process_role;               /* 进程角色 */
    char conf_file[CONF_FILE_LEN];  /* 配置文件, -f指定 */
    char signal[SIGNAL_STR_LEN];    /* 处理信号, -s指定 */
    pid_t child_pid[WORKER_NUM_MAX];/* 记录worker + 监控进程号 */
    void *conf;                     /* .conf配置信息, CFG_INFO */
    void *zones;                    /* .zone域信息, ZONES */
    void *sh_mem;                   /* 共享内存 */
}GLB_VARS;

/* 内存分配宏, 便于后续拓展 */
#define SDNS_MALLOC     malloc
#define SDNS_MEMSET     memset
#define SDNS_REALLOC    realloc 
#define SDNS_FREE       free
#define SDNS_MEMCPY     memcpy

#endif
