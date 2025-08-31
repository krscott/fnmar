cfg() {
    (
        set -eu

        rm -rf build/
        cmake -B build
    )
}

bld() {
    (
        set -eu

        if [ ! -d build ]; then
            cfg
        fi

        cmake --build build

        if [ "$CMAKE_BUILD_TYPE" = "Debug" ]; then
            mkdir -p .compile-db
            cp build/compile_commands.json .compile-db
        fi
    )
}

run() {
    (
        set -eu

        bld
        ./build/src/fnmar "$@"
    )
}

crun() {
    (
        set -eu

        cfg
        run "$@"
    )
}

debug() {
    export CMAKE_BUILD_TYPE=Debug
}

release() {
    unset CMAKE_BUILD_TYPE
}

logdebug() {
    export LOG=debug
}

loginfo() {
    export LOG=info
}

logwarn() {
    export LOG=warn
}
