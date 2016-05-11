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
int process_signals();

#endif

