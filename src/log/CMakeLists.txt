#指定源文件
aux_source_directory(.  LIB_SRCS)
list(APPEND LIB_SRCS "./statistics.c")
list(REMOVE_DUPLICATES LIB_SRCS)

#指定生成目标
add_library(log ${LIB_SRCS})
