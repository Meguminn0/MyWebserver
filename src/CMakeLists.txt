cmake_minimum_required(VERSION 2.8)

aux_source_directory(./ SRC)

include_directories(${HEAD_PATH})

link_directories(${LIB_PAHT})

set(EXECUTABLE_OUTPUT_PATH ${EXE_PATH})

# 在 /usr/lib64/mysql 或 /usr/lib/mysql 目录下，查找libmysqlclient 动态库文件
find_library(MYSQL_LIBRARY NAMES libmysqlclient.so
            PATHS /usr/lib64/mysql /usr/lib/mysql)

if(NOT MYSQL_LIBRARY)
    message(SEND_ERROR "Could not find mysql library! pleace be sure download mysql")
else()
    message(STATUS "find mysql library: ${MYSQL_LIBRARY}")
endif()

add_executable(${EXE_NAME} ${SRC})

target_include_directories(${EXE_NAME} PUBLIC /usr/include/mysql)

target_link_libraries(${EXE_NAME} pthread)