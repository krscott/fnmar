cmake_minimum_required(VERSION 3.11)
project(
    fnmar
    LANGUAGES C
)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
install(TARGETS ${PROJECT_NAME})

