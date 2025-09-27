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

release() {
    unset DISABLE_OPTIMZATIONS
    unset CMAKE_BUILD_TYPE
}

debug() {
    release
    export CMAKE_BUILD_TYPE=Debug
}

o0() {
    debug
    export DISABLE_OPTIMIZATIONS=1
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

setup_vscode() {
    mkdir -p .vscode/
    cp dev/vscode/* .vscode/
}

# Check "$1" is not a file because direnv will pass a profile when sourced
if [ $# -gt 0 ] && [ ! -e "$1" ]; then
    "$@"
    exit $?
fi

export CC=clang
debug
