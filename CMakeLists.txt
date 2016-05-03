#最低版本要求
cmake_minimum_required(VERSION 2.8)

#指定cmake自定义的查找模块脚本的目录
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#项目信息
project(smartDNS C)

#设置底层报文引擎类型
SET(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS ON)
SET(ENGINE_TYPE "DEFAULT")
#SET(ENGINE_TYPE "DPDK")

#添加编译选项
ADD_DEFINITIONS(-Wall)
ADD_DEFINITIONS(-Werror)
ADD_DEFINITIONS(-Wunused)
ADD_DEFINITIONS(-std=gnu99)

#指定Debug编译模式("$ cmake -DCMAKE_BUILD_TYPE=Debug ..")
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Debug)         #开发期间默认为Debug模式
    #set(CMAKE_BUILD_TYPE Release)      #产品最终未Release模式
endif() 
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -g -ggdb -DDNS_DEBUG")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3")

#创建定制目标, 执行外部脚本, 从而创建统计模块儿需要的信息
#<TAKE CARE!!!>由于没有输出, 此命令总是被执行
ADD_CUSTOM_TARGET(creatStat
    COMMAND "python"
    "${CMAKE_SOURCE_DIR}/create_statistics_mod.py"
    "${ENGINE_TYPE}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

#指定编译子目录, 并保证统计信息模块儿先于log编译, 因为log中会引用
#creatStat任务的结果(生成的头文件)
add_subdirectory(src)
add_dependencies(log creatStat)

#编译单元测试
find_package(Check)
if(CHECK_FOUND)
    add_subdirectory(check)
    enable_testing()
endif()

