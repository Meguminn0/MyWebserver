cmake_minimum_required(VERSION 2.8)

aux_source_directory(./ SRC)

include_directories(${HEAD_PATH})

link_directories(${LIB_PAHT})

set(EXECUTABLE_OUTPUT_PATH ${EXE_PATH})

add_executable(${EXE_NAME} ${SRC})

target_link_libraries(${EXE_NAME} pthread)