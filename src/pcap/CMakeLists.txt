#指定源文件
aux_source_directory(.  LIB_SRCS)

#指定生成目标
add_library(pcap ${LIB_SRCS})
target_link_libraries(log)
