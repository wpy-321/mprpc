cmake_minimum_required(VERSION 3.0)
project(mprpc)

set(CMAKE_BUILD_TYPE "Debug")

#设置生成可执行文件的保存路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#设置生成的库文件保存路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

# 设置项目编译头文件搜索路径 -i
include_directories(${PROJECT_SOURCE_DIR}/src/include)
include_directories(${PROJECT_SOURCE_DIR}/example)
# 设置项目库文件搜索路径 -l
link_libraries(${PROJECT_SOURCE_DIR}/lib)

# src子目录下包含了mpRpc框架的代码
add_subdirectory(src)
# example子目录下包含了使用mpRpc框架示例代码
add_subdirectory(example)


