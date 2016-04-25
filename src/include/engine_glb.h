#ifndef ENGINE_GLB_H
#define ENGINE_GLB_H

/**
 * 定义数据报文格式
 */
typedef struct st_pkt_info {
    void *eth_hdr;
    void *ip_hdr;
    void *udp_hdr;
    void *dns_hdr;
    char *cur_pos;              /* 当前处理位置 */

    union{
        uint32_t ip4;
    }src_ip;                    /* 源IP, 网络字节序 */

    char domain[DOMAIN_LEN_MAX];
    uint16_t q_type;
    uint16_t q_class;

    union {                     /* A记录, 网络字节序 */
        uint32_t ip4;
    }rr_res[RR_PER_TYPE_MAX];
    uint32_t rr_res_ttl;
    int rr_res_cnt;
}PKT_INFO;
typedef struct st_pkt {
    PKT_INFO info;              /* 信息区, 存放解析中间结果, PKT_INFO */
    char *data;                 /* 报文正文 */
    int data_len;               /* 报文长度 */
}PKT;

/**
 * 初始化报文收发引擎, 一般在主进程调用, 如DPDK环境等
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int pkt_engine_init();
/**
 * 第二阶段初始化报文收发引擎, 一般在主worker进程调用,
 * 如普通的SOCKET插口操作等
 */
int pkt_engine_init_2();

/**
 * 启动报文收发引擎, 如DPDK环境等
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int start_pkt_engine();

/**
 * 收/发报文
 * @param pkt: [in][out], 报文指针
 * @retval: RET_OK/RET_ERR
 *
 * @note
 *  1) 普通引擎报文为串行处理, 因此这两个函数应该成对出现, 并且顺序为
 *      先收后发
 */
int receive_pkt(PKT **pkt);
int send_pkt(PKT *pkt);

#endif
