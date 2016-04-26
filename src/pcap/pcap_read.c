#include "util_glb.h"
#include "pcap_glb.h"
#include "log_glb.h"
#include "pcap_read.h"

int pcap_close(FILE *fp)
{
    /* 忽略错误 */
    (void)fclose(fp);

    return RET_OK;
}

FILE *pcap_open(char *file_name)
{
    PCAP_HDR pcap_hdr = {0};
    FILE *fp;

    fp = fopen(file_name, "rb");
    if (fp == NULL) {
        SDNS_LOG_ERR("open [%s] failed, [%s]", file_name, strerror(errno));
        return NULL;
    }

    /* pcap head */
    if (fread(&pcap_hdr, sizeof(PCAP_HDR), 1, fp) != 1) {
        if (feof(fp)) {
            SDNS_LOG_ERR("end of file, [%s]", file_name);
        } else if (ferror(fp)) {
            SDNS_LOG_ERR("read header failed, [%s]", strerror(errno));
        } else {
            SDNS_LOG_ERR("open [%s] failed", file_name);
        }

        fclose(fp);
        return NULL;
    }

    if (pcap_hdr.magic_number != PCAP_MAGIC_NUMBER) {
        SDNS_LOG_ERR("Magic Number not match! [%x]/[%x]", 
                pcap_hdr.magic_number, PCAP_MAGIC_NUMBER);

        fclose(fp);
        return NULL;
    }

    return fp;
}

int pcap_read_one_pkt(FILE *fp, char *buf, int buf_len)
{
    int tmp_ret;
    PCAP_PKT_HDR pkt_hdr = {0};

    if (fp == NULL 
            || buf == NULL 
            || buf_len == 0) {
        SDNS_LOG_ERR("param err");
        return RET_ERR;
    }

    /* 读取数据包头 */
    tmp_ret = fread(&pkt_hdr, sizeof(PCAP_PKT_HDR), 1, fp);
    if (tmp_ret != 1) {
        if (feof(fp)) {
            return RET_EOF;
        } else if (ferror(fp)) {
            SDNS_LOG_ERR("read pkt header failed, [%s]", strerror(errno));
        } else {
            SDNS_LOG_ERR("read failed, [%s]", strerror(errno));
        }
        return RET_ERR;
    }

    /* 跳过特长包 */
    if (pkt_hdr.cap_len > buf_len
            || pkt_hdr.cap_len == 0) {
        SDNS_LOG_ERR("malformed pkt, [%u]/[%d]", pkt_hdr.cap_len, buf_len);
        (void)fseek(fp, pkt_hdr.cap_len, SEEK_CUR);
        return 0;
    }

    if (fread(buf, pkt_hdr.cap_len, 1, fp) !=1) {
        if (feof(fp) == 0) {
            return RET_EOF;
        } else if (ferror(fp)) {
            SDNS_LOG_ERR("read pkt data failed, [%s]", strerror(errno));
        } else {
            SDNS_LOG_ERR("read failed, [%s]", strerror(errno));
        }
        return RET_ERR;
    }


    return (int)(pkt_hdr.cap_len);
}


