cmake_minimum_required(VERSION 3.16)

add_library(krslib)

target_include_directories(krslib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_sources(krslib PRIVATE
    krs_cliopt.c
    krs_dynamic_array.c
    krs_log.c
    krs_span.c
    krs_str.c
)

# Generate prexy.h

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/prexy.h
    COMMAND prexy --header > ${CMAKE_CURRENT_SOURCE_DIR}/prexy.h
    # DEPENDS prexy
    COMMENT "Generating prexy.h"
    VERBATIM
)

target_sources(krslib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/prexy.h)
