#include <libgen.h>         /* for dirname() */
#include "util_glb.h"
#include "hiredis.h"
#include "log_glb.h"
#include "monitor_glb.h"
#include "monitor.h"

static inline int is_exist()
{
    return true;
}

static inline redisContext * connect_redis() 
{
    redisContext *c;
    struct timeval timeout = { 1, 500000 };    /* 60.5 seconds */

    c = redisConnectWithTimeout("127.0.0.1", 6379, timeout);
    if (c == NULL || c->err) {
        if (c) {
            SDNS_LOG_ERR("conn redis error: [%s]", c->errstr);
            redisFree(c);
        } else {
            SDNS_LOG_ERR("can't allocate redis context");
        }

        return NULL;
    }

    return c;
}

int save_binary_in_str(char *name, void *val, size_t len)
{
    redisContext *c;
    redisReply *reply;

    assert(name);
    assert(val);
    assert(len > 0);

    /* 连接redis服务器 */
    c = connect_redis();
    if (c == NULL) {
        return RET_ERR;
    }

    /* 存储数据 */
    reply = redisCommand(c,"SET %s %b", name, val, len);
    if (reply == NULL || reply->type == REDIS_REPLY_ERROR) {
        SDNS_LOG_ERR("save binary data err");
        redisFree(c);
        return RET_ERR;
    }
    freeReplyObject(reply);

    /* 断开连接 */
    redisFree(c);
    return RET_OK;
}

int append_binary_in_hash(char *hash, char *field, void *val, size_t len)
{
    redisContext *c;
    redisReply *orig_data_reply;
    redisReply *reply;
    bool need_add = false;
    bool need_append = true;
    int tmp_ret = RET_ERR;

    assert(hash);
    assert(field);
    assert(val);
    assert(len > 0);

    /* 连接redis服务器 */
    c = connect_redis();
    if (c == NULL) {
        return RET_ERR;
    }

    /* 是否需要存储??? */
    orig_data_reply = redisCommand(c,"HGET %s %s", hash, field);
    if (orig_data_reply == NULL 
            || orig_data_reply->type == REDIS_REPLY_ERROR) {
        SDNS_LOG_ERR("get [%s]:[%s] failed", hash, field);
        goto EXITED;
    }
    if (orig_data_reply->type == REDIS_REPLY_NIL) {
        SDNS_LOG_DEBUG("create [%s]:[%s]", hash, field);
        need_add = true;
    } else if (orig_data_reply->type == REDIS_REPLY_STRING) {
        for (int i=0; i<orig_data_reply->len; i+=len) {
            if (SDNS_MEMCMP(orig_data_reply->str + i, val, len) == 0) {
                need_append = false;
                break;
            }
        }
    } else {
        SDNS_LOG_ERR("reply type not support [%d]", orig_data_reply->type);
        goto EXITED;
    }

    if (need_add) {
        reply = redisCommand(c,"HSET %s %s %b", hash, field, val, len);
    } else if (need_append) {
        reply = redisCommand(c,"HSET %s %s %b%b", hash, field,
                orig_data_reply->str, orig_data_reply->len, val, len);
    } else {
        tmp_ret = RET_OK;
        goto EXITED;
    }

    if (reply == NULL
            || reply->type == REDIS_REPLY_ERROR) {
        SDNS_LOG_ERR("append binary data err, [%s]:[%s]", hash, field);
        goto EXITED;
    }

    tmp_ret = RET_OK;

EXITED:
    if (orig_data_reply) {
        freeReplyObject(orig_data_reply);
    }
    if (reply) {
        freeReplyObject(reply);
    }
    redisFree(c);
    return tmp_ret;
}

int append_binary_to_str(char *name, void *val, size_t len)
{
    redisContext *c;
    redisReply *orig_data_reply = NULL;
    redisReply *reply = NULL;
    bool need_add = false;
    bool need_append = true;
    int tmp_ret = RET_ERR;

    assert(name);
    assert(val);
    assert(len > 0);
    
    /* 连接redis服务器 */
    c = connect_redis();
    if (c == NULL) {
        return RET_ERR;
    }

    /* 是否需要存储??? */
    orig_data_reply = redisCommand(c,"GET %s", name);
    if (orig_data_reply == NULL 
            || orig_data_reply->type == REDIS_REPLY_ERROR) {
        SDNS_LOG_ERR("get [%s] failed", name);
        goto EXITED;
    }
    if (orig_data_reply->type == REDIS_REPLY_NIL) {
        SDNS_LOG_DEBUG("create [%s]", name);
        need_add = true;
    } else if (orig_data_reply->type == REDIS_REPLY_STRING) {
        for (int i=0; i<orig_data_reply->len; i+=len) {
            if (SDNS_MEMCMP(orig_data_reply->str + i, val, len) == 0) {
                need_append = false;
                break;
            }
        }
    } else {
        SDNS_LOG_ERR("reply type not support [%d]", orig_data_reply->type);
        goto EXITED;
    }

    if (need_add) {
        reply = redisCommand(c,"SET %s %b", name, val, len);
    } else if (need_append) {
        reply = redisCommand(c,"SET %s %b%b", name,
                orig_data_reply->str, orig_data_reply->len, val, len);
    } else {
        tmp_ret = RET_OK;
        goto EXITED;
    }

    if (reply == NULL
            || reply->type == REDIS_REPLY_ERROR) {
        SDNS_LOG_ERR("append binary data err, [%s]", name);
        goto EXITED;
    }

    tmp_ret = RET_OK;

EXITED:
    if (orig_data_reply) {
        freeReplyObject(orig_data_reply);
    }
    if (reply) {
        freeReplyObject(reply);
    }
    redisFree(c);
    return tmp_ret;
}

int traverse_binary_str(char *name, binary_mem_dispose func, size_t len)
{
    redisContext *c;
    redisReply *reply;
    void *data;

    assert(name);
    assert(func);
    assert(len > 0);

    c = connect_redis();
    if (c == NULL) {
        return RET_ERR;
    }

    reply = redisCommand(c,"GET %s", name);
    if (reply == NULL 
            || reply->type == REDIS_REPLY_ERROR
            || reply->type == REDIS_REPLY_NIL) {
        SDNS_LOG_WARN("NO data, just skip!!!");
        goto SKIP;
    } else if (reply->type != REDIS_REPLY_STRING) {
        SDNS_LOG_WARN("data type err");
        goto FAILED;
    }
    data = reply->str;
    for (int i = 0; i<reply->len; i+=len) {
        if (i+len > reply->len) {
            SDNS_LOG_WARN("data overstack???");
            break;
        }
        if (func(data + i) == RET_ERR) {
            goto FAILED;
        }
    }

    freeReplyObject(reply);
SKIP:
    redisFree(c);
    return RET_OK;
FAILED:
    freeReplyObject(reply);
    redisFree(c);
    return RET_ERR;
}

int traverse_hash_binary_str(char *hash, char *field, 
        binary_mem_dispose func, size_t len)
{
    redisContext *c;
    redisReply *reply;
    void *data;

    assert(hash);
    assert(field);
    assert(func);
    assert(len > 0);

    c = connect_redis();
    if (c == NULL) {
        return RET_ERR;
    }

    reply = redisCommand(c,"HGET %s %s", hash, field);
    if (reply == NULL 
            || reply->type == REDIS_REPLY_ERROR
            || reply->type == REDIS_REPLY_NIL) {
        SDNS_LOG_WARN("NO data, just skip!!!");
        goto SKIP;
    } else if (reply->type != REDIS_REPLY_STRING) {
        SDNS_LOG_WARN("data type err");
        goto FAILED;
    }
    data = reply->str;
    for (int i = 0; i<reply->len; i+=len) {
        if (i+len > reply->len) {
            SDNS_LOG_WARN("data overstack???");
            break;
        }
        if (func(data + i) == RET_ERR) {
            goto FAILED;
        }
    }

    freeReplyObject(reply);
SKIP:
    redisFree(c);
    return RET_OK;
FAILED:
    freeReplyObject(reply);
    redisFree(c);
    return RET_ERR;
}

int dispose_binary_in_str(char *name, binary_mem_dispose func)
{
    redisContext *c;
    redisReply *reply;
    void *data;

    assert(name);
    assert(func);

    c = connect_redis();
    if (c == NULL) {
        return RET_ERR;
    }

    reply = redisCommand(c,"GET %s", name);
    if (reply == NULL 
            || reply->type == REDIS_REPLY_ERROR
            || reply->type == REDIS_REPLY_NIL) {
        SDNS_LOG_WARN("NO data, just skip!!!");
        goto SKIP;
    } else if (reply->type != REDIS_REPLY_STRING) {
        SDNS_LOG_WARN("data type err");
        goto FAILED;
    }
    data = reply->str;
    if (func(data) == RET_ERR) {
        goto FAILED;
    }

    freeReplyObject(reply);
    redisFree(c);
    return RET_OK;
FAILED:
    freeReplyObject(reply);
SKIP:
    redisFree(c);
    return RET_ERR;
}

int start_monitor()
{
    pid_t child_pid;

    /* 启动redis进程, 存储解析的配置信息 */
    child_pid = fork();
    if (child_pid) {
        SDNS_LOG_DEBUG("In parent, fork redis[%d], waiting...", child_pid);
        sleep(5);       /* magic 5: 等待redis启动 */
        SET_CHILD_PROCESS(PROCESS_ROLE_MONITOR, child_pid);
        SDNS_LOG_DEBUG("DONE, continue parse");
    } else {
        SDNS_LOG_DEBUG("starting redis...");
        SET_PROCESS_ROLE(PROCESS_ROLE_MONITOR);

        char tmp_conf_buf[255];     /* magic 255: 文件绝对路径名长度上限 */
        char tmp_bin_buf[255];
        int tmp_ret;

        tmp_ret = snprintf(tmp_conf_buf, sizeof(tmp_conf_buf), "%s", 
                get_glb_vars()->conf_file);
        tmp_ret = strlen(dirname(tmp_conf_buf));
        snprintf(tmp_conf_buf + tmp_ret, sizeof(tmp_conf_buf) - tmp_ret, 
                "/%s", REDIS_SERVER_CONF);

        tmp_ret = readlink("/proc/self/exe", tmp_bin_buf, 
                sizeof(tmp_bin_buf));
        if (tmp_ret == -1) {
            SDNS_LOG_ERR("get exe path failed!!! [%s]", strerror(errno));
            return RET_ERR;
        }
        tmp_bin_buf[sizeof(tmp_bin_buf) - 1] = 0;
        tmp_ret = strlen(dirname(tmp_bin_buf));
        tmp_ret = snprintf(tmp_bin_buf + tmp_ret, 
                sizeof(tmp_bin_buf) - tmp_ret, "/%s", REDIS_SERVER_BIN); 
        if (execlp(tmp_bin_buf, tmp_conf_buf, (char *)0)< 0) {
            SDNS_LOG_ERR("start redis failed!!! [%s]/[%s]/[%s]",
                    strerror(errno), tmp_bin_buf, tmp_conf_buf);
        }
        return RET_ERR;
    }

    return RET_OK;
}
