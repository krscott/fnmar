cmake_minimum_required(VERSION 3.11)
project(
    fnmar
    LANGUAGES C
)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(src)


if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(
        ${PROJECT_NAME} PRIVATE -Weverything

        # Based on project rules
        -Wno-declaration-after-statement
        -Wno-unsafe-buffer-usage
        -Wno-pre-c11-compat
        -Wno-covered-switch-default

        # Occasionally enable for manual review
        -Wno-padded
        -Wno-switch-default
        -Wno-used-but-marked-unused
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()
install(TARGETS ${PROJECT_NAME})

