#ifndef SIGNAL_GLB_H
#define SIGNAL_GLB_H

/**
 * 设置/阻断/等待信号处理句柄
 * @param void
 * @retval: RET_OK/RET_ERR
 */
int set_required_signal(void);
int block_required_signal(void);
int wait_required_signal(void);

/**
 * 清楚信号屏蔽, 即开始接收信号
 * @param void
 * @retval: RET_OK/RET_ERR
 */
int clear_mask_signal(void);

/**
 * 处理进程信号
 * @param: void
 * @retval: RET_OK/RET_ERR
 */
int process_signals(void);

/**
 * 清理子进程
 */
void kill_child_all(void);

/**
 * 处理传入参数-s
 */
void process_option_signal(void);

#endif

