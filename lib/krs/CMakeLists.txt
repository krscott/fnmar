cmake_minimum_required(VERSION 3.11)

add_library(krslib)

target_include_directories(krslib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(krslib PRIVATE
    krs_str.c
    krs_dynamic_array.c
)


