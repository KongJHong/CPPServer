# 最低CMake版本要求
cmake_minimum_required(VERSION 3.10.0)

# 项目名称
project(ServerDemo)

# 设置C/C++标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CXX_FLAGS 
        -Wall
        -std=c++11
        -lpthread
        -Wno-unused-parameter
        -03
        )
link_libraries(pthread)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS_DEBUG "-O3") #设置C++编译选项，也可以通过指令ADD_DEFINITIONS()添加。
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")

# 头文件路径
include_directories("Inc")

# 枚举头文件
file(GLOB_RECURSE INCLUDES "Inc/*.h" "Inc/*.hpp")

# 枚举源文件
aux_source_directory("Src" SOURCES)

# 输出路径 EXECUTABLE_OUTPUT_PATH：系统输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)

# 生成可执行的文件
add_executable(${PROJECT_NAME} ${SOURCES} ${INCLUDES})
