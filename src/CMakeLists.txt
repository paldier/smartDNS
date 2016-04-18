#指定源文件
aux_source_directory(. MAIN_SRCS)

#指定头文件
include_directories(${CMAKE_SOURCE_DIR}/src/include)

#指定编译子目录
add_subdirectory(master)
add_subdirectory(engine)
add_subdirectory(log)

#指定生成目标
add_executable(smartDNS ${MAIN_SRCS})
target_link_libraries(smartDNS master engine log)



