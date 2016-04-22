#指定源文件
file(GLOB CHECK_FILES "${CMAKE_CURRENT_SOURCE_DIR}/check_*.c")
list(APPEND CHECK_SRCS ${CHECK_FILES})

#包含头文件
include_directories(${CMAKE_SOURCE_DIR}/src/include)
include_directories(${CMAKE_SOURCE_DIR}/src/master)
include_directories(${CMAKE_SOURCE_DIR}/src/worker)
include_directories(${CMAKE_SOURCE_DIR}/src/zone)
include_directories(${CMAKE_SOURCE_DIR}/src/pcap)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_definitions(-DDNS_CK_NOFORK)
endif()
#指定生成目标
add_executable(check_smartDNS ${CHECK_SRCS})
target_link_libraries(check_smartDNS  
    ${CHECK_LIBRARIES} 
    master 
    worker
    zone
    pcap
    )

#启用测试
enable_testing()
add_test(NAME check_main COMMAND check_smartDNS)



