#指定源文件
file(GLOB CHECK_FILES "${CMAKE_CURRENT_SOURCE_DIR}/check_*.c")
list(APPEND CHECK_SRCS ${CHECK_FILES})

#包含头文件
include_directories(${CMAKE_SOURCE_DIR}/src/include)
include_directories(${CMAKE_SOURCE_DIR}/src/master)

#指定生成目标
add_executable(check_smartDNS ${CHECK_SRCS})
target_link_libraries(check_smartDNS  ${CHECK_LIBRARIES} master)

#启用测试
enable_testing()
add_test(NAME check_main COMMAND check_smartDNS)



