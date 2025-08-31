cmake_minimum_required(VERSION 3.16)

add_library(krslib)

target_include_directories(krslib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(krslib PRIVATE
    krs_dynamic_array.c
    krs_log.c
    krs_str.c
)


