cmake_minimum_required(VERSION 3.5)
 
project(file_index)

find_package(Boost 1.69 REQUIRED COMPONENTS
    filesystem)

find_package(GTest REQUIRED)

add_executable(file_index
    lmdb-safe/lmdb-safe.cc
    bucket_cache.cpp
    main.cpp)

target_include_directories(file_index PUBLIC
    "${CMAKE_SOURCE_DIR}/fmt/include"
    "${CMAKE_SOURCE_DIR}/lmdb-safe")

target_link_libraries(file_index
    xxhash lmdb gtest pthread)

set_property(TARGET file_index PROPERTY CXX_STANDARD 20)
