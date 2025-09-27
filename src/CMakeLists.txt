add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} krslib)

# target_sources(${PROJECT_NAME} PRIVATE
#     file.c
# )

if(WIN32)
    target_link_libraries(${PROJECT_NAME} shlwapi)
endif()

# Generate *_prexy.h files

set(PREXY_FILES
    main.c
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

    target_sources(${PROJECT_NAME} PRIVATE ${h_path})
endforeach()

