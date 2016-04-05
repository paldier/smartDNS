#最低版本要求
cmake_minimum_required(VERSION 2.8)

#指定cmake自定义的查找模块脚本的目录
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

#项目信息
project(smartDNS)

#添加编译选项
ADD_DEFINITIONS(-Wall)
ADD_DEFINITIONS(-Werror)
ADD_DEFINITIONS(-Wunused)
ADD_DEFINITIONS(-std=gnu99)

#指定Debug编译模式("$ cmake -DCMAKE_BUILD_TYPE=Debug ..")
if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release)
endif() 
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -g -ggdb")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3")

#指定编译子目录
add_subdirectory(src)

#编译单元测试
find_package(Check)
if(CHECK_FOUND)                                                                    
    add_subdirectory(check)
    enable_testing()                                                                 
endif()

