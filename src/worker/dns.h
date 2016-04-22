#ifndef DNS_H
#define DNS_H


#define DOT_ASCII_VAL   '.'                 /* for . */

#define RR_TYPE_A       htons(1)
#define RR_TYPE_NS      htons(2)
#define RR_TYPE_CNAME   htons(5)

/**
 * DNS头部
 */
#define DNS_HDR_LEN     sizeof(DNS_HDR)
typedef struct st_dns_header {
    uint16_t tran_id;
    uint16_t flags;
    uint16_t q_cnt;
    uint16_t an_cnt;
    uint16_t ns_cnt;
    uint16_t ar_cnt;
}__attribute__((packed)) DNS_HDR;

/**
 * DNS请求信息示意结构, 用于queries
 */
typedef struct st_query {
    char domain_name[0];
    uint16_t type;
    uint16_t rr_class;
}__attribute__((packed)) DNS_QUERY;

/**
 * DNS应答信息示意结构, 用于Answer/Authoritative ns/Additional records
 */
typedef struct st_answer {
    char domain_name[0];
    uint16_t type;
    uint16_t rr_class;
    uint32_t ttl;
    uint16_t data_len;
    char data[0];
}__attribute__((packed)) DNS_AN;


/**
 * 获取DNS请求的域名, 格式www.baidu.com.
 * @param beg: [in], 域名开始的字节指针
 * @param len: [in], 可解析的报文总长度
 * @param res: [in][out], 提取的域名
 * @retval: 为获取域名扫描过的字节数/RET_ERR
 *
 * @NOTE
 *  1) 此函数处理无压缩域名
 */
int get_query_domain(char *beg, int len, char *res);

#endif
