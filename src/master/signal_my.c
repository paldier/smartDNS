#include <signal.h>             /* for sigaction/sigemptyset() */
#include <sys/wait.h>           /* for waitpid() */
#include "util_glb.h"
#include "signal_glb.h"
#include "log_glb.h"
#include "signal_my.h"

static int s_received_signal;       /* 收到的信号 */
static sigset_t s_set;              /* 处理的信号集合 */

/**
 * 定义程序处理的信号, 及信号处理函数
 */
static struct st_signal {
    int  signo;         /* 信号 */
    char *signame;      /* 信号名 */
    char *name;         /* 在程序中的特殊含义 */
    void (*handler)(int signo);
    int (*spec_handler)(GLB_VARS *glb_vars);
} SIGNALS_ARR [] = {
    {SIGCHLD, "SIGCHLD", "", smartDNS_signal_handler, process_SIGCHLD},
    {0, NULL, NULL, NULL, NULL}
};

void smartDNS_signal_handler(int signo)
{
    struct st_signal  *sig;

    for (sig = SIGNALS_ARR; sig->signo != 0; sig++) {
        if (sig->signo == signo) {
            s_received_signal = signo;
            break;
        }
    }

    /* 记录当前不支持的信号 */
    if (sig->signo == 0) {
        SDNS_LOG_WARN("recv signal [%s]", sig->signame);
    }
}

int process_SIGCHLD(GLB_VARS *glb_vars)
{
    int  status;
    pid_t child_pid;

    for (;;) {
        child_pid = waitpid(-1, &status, WNOHANG);

        /* 没有退出的子进程 */
        if (child_pid == 0) {
            return RET_OK;
        }

        /* 错误处理 */
        if (child_pid == -1) {
            /* WNOHANG was not set and an unblocked signal or a SIGCHLD 
             * was caught */
            if (errno == EINTR) {
                continue;
            }

            SDNS_LOG_WARN("[%d] %s", errno, strerror(errno));
            return RET_ERR;
        }

        /* 捕获到结束的子进程 */
        if (WIFEXITED(status)) {
            SDNS_LOG_WARN("child[%d] exited, status=%d", 
                    child_pid, WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            SDNS_LOG_WARN("child[%d] killed by signal %d", 
                    child_pid, WTERMSIG(status));
        } else if (WIFSTOPPED(status)) {
            SDNS_LOG_WARN("child[%d] stopped by signal %d", 
                    child_pid, WSTOPSIG(status));
        } else if (WIFCONTINUED(status)) {
            SDNS_LOG_WARN("child[%d] continued", child_pid);
        }
    }

    return RET_OK;
}

int set_required_signal()
{
    struct st_signal  *sig;
    struct sigaction   sa;

    sigemptyset(&s_set);

    for (sig = SIGNALS_ARR; sig->signo != 0; sig++) {
        SDNS_MEMSET(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = sig->handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(sig->signo, &sa, NULL) == -1) {
            SDNS_LOG_ERR("set [%s] failed", sig->signame);
            return RET_ERR;
        }

        sigaddset(&s_set, sig->signo);
    }

    return RET_OK;
}

int block_required_signal()
{
    if (sigprocmask(SIG_BLOCK, &s_set, NULL) == -1) {
        SDNS_LOG_ERR("mask signal failed");
        return RET_ERR;
    }

    return RET_OK;
}

int clear_mask_signal()
{
    sigset_t tmp_set;

    if (sigemptyset(&tmp_set) == -1) {
        SDNS_LOG_ERR("%s", strerror(errno));
        return RET_ERR;
    }

    if (sigprocmask(SIG_SETMASK, &tmp_set, NULL) == -1) {
        SDNS_LOG_ERR("clear mask failed");
        return RET_ERR;
    }

    return RET_OK;
}

int wait_required_signal()
{
    sigset_t tmp_set;

    if (sigemptyset(&tmp_set) == -1) {
        SDNS_LOG_ERR("%s", strerror(errno));
        return RET_ERR;
    }

    /* 0) 此函数调用后, 利用tmp_set代替调用前的s_set, 相当于不屏蔽任何信号
     * 1) 进程截获信号后, 由handler先处理, 然后此函数再返回, 参考man
     * 2) 并且此函数返回后, 恢复调用此函数时进程的信号屏蔽
     * 3) 此函数基本返回-1, 并且errno设置为EINTR
     * 4) 如果信号是terminate进程, 此函数不返回 */
    (void)sigsuspend(&tmp_set);

    return RET_OK;
}

int process_signals(GLB_VARS *glb_vars)
{
    struct st_signal  *sig;

    for (sig = SIGNALS_ARR; sig->signo != 0; sig++) {
        if (sig->signo == s_received_signal
                && sig->spec_handler) {
            if (sig->spec_handler(glb_vars) == RET_ERR) {
                SDNS_LOG_WARN("[%s] failed", sig->signame);
                return RET_ERR;
            }
            break;
        }
    }

    return RET_OK;
}

