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

#指定编译结果生成路径
SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

#创建定制目标, 执行外部脚本, 从而创建统计模块儿需要的信息
#<TAKE CARE!!!>由于没有输出, 此命令总是被执行
ADD_CUSTOM_TARGET(creatStat
    COMMAND "python"
    "${CMAKE_SOURCE_DIR}/create_statistics_mod.py"
    "${ENGINE_TYPE}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

#创建定制目标, 编译依赖对象
set(DEPS_INCLUDE "${CMAKE_BINARY_DIR}/deps/include")
set(DEPS_LIB "${CMAKE_BINARY_DIR}/deps/lib")
set(DEPS_BIN "${CMAKE_BINARY_DIR}/bin")
file(MAKE_DIRECTORY ${DEPS_LIB})
file(MAKE_DIRECTORY ${DEPS_INCLUDE})
file(MAKE_DIRECTORY ${DEPS_BIN})
link_directories(${DEPS_LIB})
include_directories(${DEPS_INCLUDE})

set(REDIS_SRC "${CMAKE_SOURCE_DIR}/deps/redis")
set(REDIS_SERVER "redis-server")
set(REDIS_CLI "libhiredis.a")
set(REDIS_CLI_H "hiredis.h")
if((NOT EXISTS ${REDIS_SERVER_DST}) OR (NOT EXISTS ${REDIS_CLI_DST}))
    message(STATUS "building ${REDIS_SERVER_SRC}")

    execute_process(
        COMMAND make distclean
        WORKING_DIRECTORY ${REDIS_SRC})
    execute_process(
        COMMAND make -j 8
        WORKING_DIRECTORY ${REDIS_SRC})
    execute_process(
        COMMAND cp src/${REDIS_SERVER}  ${DEPS_BIN}
        WORKING_DIRECTORY ${REDIS_SRC})
    execute_process(
        COMMAND cp deps/hiredis/${REDIS_CLI}  ${DEPS_LIB}
        WORKING_DIRECTORY ${REDIS_SRC})
    execute_process(
        COMMAND cp deps/hiredis/${REDIS_CLI_H}  ${DEPS_INCLUDE}
        WORKING_DIRECTORY ${REDIS_SRC})
    
    message(STATUS "done")
endif()

set(GEOIP_DB "GeoLite2-City.mmdb")
if(NOT EXISTS ${DEPS_BIN}/${GEOIP_DB})
    message(STATUS "unzip ${GEOIP_DB}.gz ...")

    execute_process(
        COMMAND cp ./deps/GeoLite2-City.mmdb.gz ${DEPS_BIN}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
    execute_process(
        COMMAND gunzip GeoLite2-City.mmdb.gz
        WORKING_DIRECTORY ${DEPS_BIN})

    message(STATUS "done")
endif()

set(GEOIP_LIB_SRC "${CMAKE_SOURCE_DIR}/deps/geoip2")
set(GEOIP_LIB "libmaxminddb.a")
set(GEOIP_LIB_H "maxminddb.h")
if(NOT EXISTS ${DEPS_LIB}/${GEOIP_LIB})
    message(STATUS "building ${GEOIP_LIB} ...")

    execute_process(
        COMMAND git submodule init 
        WORKING_DIRECTORY ${GEOIP_LIB_SRC})
    execute_process(
        COMMAND git submodule update
        WORKING_DIRECTORY ${GEOIP_LIB_SRC})
    execute_process(
        COMMAND ./bootstrap
        WORKING_DIRECTORY ${GEOIP_LIB_SRC})
    execute_process(
        COMMAND ./configure
        WORKING_DIRECTORY ${GEOIP_LIB_SRC})
    execute_process(
        COMMAND make
        WORKING_DIRECTORY ${GEOIP_LIB_SRC})
    execute_process(
        COMMAND cp ./src/.libs/${GEOIP_LIB} ${DEPS_LIB} 
        WORKING_DIRECTORY ${GEOIP_LIB_SRC})
    execute_process(
        COMMAND cp ./include/${GEOIP_LIB_H} ${DEPS_INCLUDE} 
        WORKING_DIRECTORY ${GEOIP_LIB_SRC})

    message(STATUS "done")
endif()
#指定编译子目录, 并保证统计信息模块儿先于log编译, 因为log中会引用
#creatStat任务的结果(生成的头文件)
add_subdirectory(src)

#编译单元测试
find_package(Check)
if(CHECK_FOUND)
    add_subdirectory(check)
    enable_testing()
endif()

