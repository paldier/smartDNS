#ifndef CHECK_MAIN_H
#define CHECK_MAIN_H

#include <check.h>
#include "check_util.h"

/* 宏: 简化测试集注册流程 */
#define REGISTER_SUITE(t) \
    static void __attribute__((constructor, used)) test_suite_##t(void) \
    {\
        add_suite(t);\
    }

/* 定义函数指针: 创建测试suite的函数原型 */
typedef Suite * (*create_suite_callback)(void);

/**
 * 添加测试suite到主进程
 * @param callback: [in], 回调函数, 用于产生测试集
 * @retval: void
 */
void add_suite(create_suite_callback callback);



#endif
