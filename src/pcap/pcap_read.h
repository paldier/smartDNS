#ifndef PCAP_READ_H
#define PCAP_READ_H

/**
 * 打开.pcap文件
 * @param file_name: [in], 文件名
 * @retval: FILE*, NULL/打开的文件描述符
 */
FILE *pcap_open(char *file_name);

/**
 * 关闭pcap处理流
 * @param fp: [in], 文件流
 * @retval: RET_OK/RET_ERR
 */
int pcap_close(FILE *fp);

/**
 * 从.pcap文件读取一个报文
 * @param fp: [in], 文件流描述符
 * @param buf: [in][out], 存放读取的报文内容的缓存
 * @param buf_len: [in], 缓存长度
 * @retval: RET_ERR/RET_EOF/报文长度(>0)
 *
 * @NOTE
 *  1) 遇到过长报文, 直接跳过, 返回值为0
 *  2) RET_ERR/RET_EOF时, 都应该停止调用此函数的读取循环
 */
#define RET_EOF (RET_ERR - 1)
int pcap_read_one_pkt(FILE *fp, char *buf, int buf_len);

#endif
