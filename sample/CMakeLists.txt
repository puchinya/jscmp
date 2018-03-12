cmake_minimum_required(VERSION 3.9)
project(jscmp_sample)

include_directories(../include)
add_executable(jscmp_sample main.c jscmp_dump.c fstream.c)
target_link_libraries(jscmp_sample jscmp)
