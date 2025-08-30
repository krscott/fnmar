#!/usr/bin/env sh
set -eu
cd "$(dirname "$(readlink -f -- "$0")")"

cmd=${1:-build}

if [ $# -gt 0 ]; then
    shift
fi

if [ -n "${DEBUG:-}" ]; then
    export CMAKE_BUILD_TYPE=Debug
fi

appname=fnmar

usage() {
    echo "Usage: ./mk.sh [configure|build|run|clean]"
}

configure() {
    (
        set -x
        cmake -B build
    )
}

build() {
    if ! [ -d build ]; then
        configure
    fi

    (
        set -x
        cmake --build build
    )

    # Only copy over debug compile_commands.json--Release uses unity build
    if [ -n "${DEBUG:-}" ] && [ -f build/compile_commands.json ]; then
        mkdir -p .compile-db
        cp build/compile_commands.json .compile-db
    fi
}

run() {
    build
    "./build/src/$appname" "$@"
}

clean() {
    rm -rf build/
}

case "$cmd" in
configure)
    configure
    ;;

build)
    build
    ;;

run)
    run "$@"
    ;;

clean)
    clean
    ;;

*)
    usage
    ;;
esac
