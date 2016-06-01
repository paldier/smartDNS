#指定源文件
aux_source_directory(. MAIN_SRCS)

#指定头文件
include_directories(${CMAKE_SOURCE_DIR}/src/include)

#指定编译子目录
add_subdirectory(master)
add_subdirectory(engine)
add_subdirectory(worker)
add_subdirectory(zone)
add_subdirectory(sort)
add_subdirectory(pcap)
add_subdirectory(dns)
add_subdirectory(monitor)
add_subdirectory(log)

add_dependencies(master creatStat)
add_dependencies(engine creatStat)
add_dependencies(worker creatStat)
add_dependencies(zone creatStat)
add_dependencies(sort creatStat)
add_dependencies(pcap creatStat)
add_dependencies(dns creatStat)
add_dependencies(monitor creatStat)
add_dependencies(log creatStat)
#指定生成目标
add_executable(smartDNS ${MAIN_SRCS})
target_link_libraries(smartDNS master engine worker zone sort log dns monitor)



