cmake_minimum_required(VERSION 3.9.6)
project(OperatingSystems2 C)

set(CMAKE_C_STANDARD 99)

add_executable(OperatingSystems2 p3150133-p3160026-prodcons-common.c p3150133-p3160026-prodcons.h p3150133-p3160026-prodcons2.c)

target_link_libraries(OperatingSystems2 pthread)