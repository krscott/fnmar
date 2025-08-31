add_executable(${PROJECT_NAME} main.c)
target_link_libraries(${PROJECT_NAME} krslib)

# target_sources(${PROJECT_NAME} PRIVATE
#     file.c
# )

if(WIN32)
    target_link_libraries(${PROJECT_NAME} shlwapi)
endif()
