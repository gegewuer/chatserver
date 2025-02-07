#入口

cmake_minimum_required(VERSION 3.5) #cmake要求最小版本
project(chat) #工程名

#配置编译选项
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g ) #-g可调试 非必需

#配置最终的可执行文件输出的路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)#非必需 不设置默认会将可执行文件放在与源文件对应的build(构建文件)中



#配置头文件的搜索路径  非必需但很有用
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/include/server)
include_directories(${PROJECT_SOURCE_DIR}/include/server/db)
include_directories(${PROJECT_SOURCE_DIR}/include/server/model)
include_directories(${PROJECT_SOURCE_DIR}/include/server/redis)
#include_directories(${PROJECT_SOURCE_DIR}/thirdparty)我自己加的，加对了
include_directories(${PROJECT_SOURCE_DIR}/thirdparty)

#加载子目录 简单项目非必需 大型项目很有用
add_subdirectory(src)