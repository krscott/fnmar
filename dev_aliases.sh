cfg() {
    rm -rf build/
    cmake -B build
}

bld() {
    cmake --build build

    if [ -f build/compile_commands.json ]; then
        mkdir -p .compile-db
        cp build/compile_commands.json .compile-db
    fi
}

run() {
    bld
    ./build/src/fnmar "$@"
}

debug() {
    export CMAKE_BUILD_TYPE=Debug
}

release() {
    unset CMAKE_BUILD_TYPE
}
