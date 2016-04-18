#指定源文件
IF (ENGINE_TYPE STREQUAL "DPDK")
    SET(LIB_SRCS engine_dpdk.c)
    list(APPEND LIB_SRCS engine_dpdk.h)
ELSEIF (ENGINE_TYPE STREQUAL "DEFAULT")
    SET(LIB_SRCS engine.c engine.h)
ENDIF()

#指定生成目标
add_library(engine ${LIB_SRCS})



