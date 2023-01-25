cmake_minimum_required(VERSION 3.5)
 
project(file_index)
 
add_executable(file_index main.cpp)
target_link_libraries(file_index lmdb)
