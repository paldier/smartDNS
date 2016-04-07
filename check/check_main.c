#include <stdio.h>      /* for printf() */
#include <stdlib.h>     /* for exit() */
#include "check_main.h"

/* 测试实例 */
static SRunner *s_sr = NULL;

void add_suite(create_suite_callback callback)
{
    if (s_sr == NULL) {
        s_sr = srunner_create(callback());
        if (s_sr == NULL) {
            perror("SRunner created failed\n");
            exit(1);
        }
    } else {
        srunner_add_suite(s_sr, callback());
    }
}


int main(int argc, char **argv)
{
    int number_failed = 0;

    if (s_sr != NULL) {
        /* set log out, locate at ~/build/check/test.log */
        srunner_set_log(s_sr, "test.log");

        /* run all suite */
        //srunner_set_fork_status(s_sr, CK_NOFORK);
        srunner_run_all(s_sr, CK_NORMAL);
        number_failed = srunner_ntests_failed(s_sr);

        /* free mem */
        srunner_free(s_sr);
    }

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
