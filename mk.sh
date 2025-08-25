#!/usr/bin/env sh
set -eu
cd "$(dirname "$(readlink -f -- "$0")")"

cmd=${1:-build}

if [ $# -gt 0 ]; then
    shift
fi

CMAKE_ARGS=${CMAKE_ARGS:--DCMAKE_BUILD_TYPE=Debug}

appname=fnmar

usage() {
    echo "Usage: ./mk.sh [configure|build|run|clean]"
}

configure() {
    (
        set -x
        cmake -B build $CMAKE_ARGS
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
