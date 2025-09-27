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
    OUTPUT prexy.h
    COMMAND prexy --header > ${CMAKE_CURRENT_SOURCE_DIR}/prexy.h
    VERBATIM
)
target_sources(krslib PRIVATE prexy.h)

# Generate *_prexy.h files

set(PREXY_FILES
    krs_cliopt.c
)

foreach(src IN LISTS PREXY_FILES)
    get_filename_component(name ${src} NAME_WE)   # filename without extension

    set(c_path ${CMAKE_CURRENT_SOURCE_DIR}/${src})
    set(h_path ${CMAKE_CURRENT_SOURCE_DIR}/${name}_prexy.h)

    add_custom_command(
        OUTPUT ${h_path}
        COMMAND prexy "${c_path}" -o "${h_path}"
        DEPENDS ${c_path}
        VERBATIM
    )

    target_sources(krslib PRIVATE ${h_path})
endforeach()

