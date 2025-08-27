add_executable(${PROJECT_NAME} main.c)

target_sources(${PROJECT_NAME} PRIVATE
    str.c
    dynamic_array.c
)

