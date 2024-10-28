cmake_minimum_required(VERSION 3.5)

# 使用 GCC
# set(CMAKE_C_COMPILER "gcc")
# set(CMAKE_CXX_COMPILER "g++")

# # 使用 Clang
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

set(ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR})

set(PRODUCT_NAME "kwlf")
project(${PRODUCT_NAME} LANGUAGES CXX C) 

set(SRC_PATH ${ROOT_PATH}/src)

file(GLOB_RECURSE SRC_FILES "${SRC_PATH}/*.c")

include_directories(
  ${SRC_PATH}
)

set(ALL_SRC_FILES
    ${SRC_FILES}
    ${ROOT_PATH}/main.c
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

add_executable(${PRODUCT_NAME} ${ALL_SRC_FILES})

target_compile_options(${PRODUCT_NAME} PRIVATE -fno-elide-constructors)