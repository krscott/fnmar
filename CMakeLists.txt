cmake_minimum_required(VERSION 3.16)
project(
    fnmar
    LANGUAGES C
)

if (CMAKE_C_COMPILER_ID STREQUAL "Clang")
    add_compile_options(
        -Weverything

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
    add_compile_options(-Wall -Wextra -pedantic)
endif()

if (CMAKE_BUILD_TYPE MATCHES "Debug")
    message("Debug build")
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

    add_compile_options(-fsanitize=undefined -fsanitize=address)
    add_link_options(-fsanitize=undefined -fsanitize=address)
else()
    message("Release build")
    set(CMAKE_UNITY_BUILD ON)

    add_compile_options(-Werror -Os)
endif()

add_subdirectory(lib/krs)
add_subdirectory(src)

install(TARGETS ${PROJECT_NAME})

