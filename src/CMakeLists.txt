cmake_minimum_required(VERSION 3.9)
project(jscmp)

include_directories(../include)
add_library(jscmp jscmp.c iostream.c)

