cmake_minimum_required(VERSION 2.8)

# 工程名
project(MyWebServer)

# 所有头文件所在目录
set(HEAD_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)

# 第三方依赖生成目录
set(LIB_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# 可执行程序的生成目录
set(EXE_PATH ${CMAKE_CURRENT_SOURCE_DIR})

# 可执行程序的名称
set(EXE_NAME mywebserver)

# 递归执行 CMakeLists.txt 文件
add_subdirectory(src)
