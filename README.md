# fnmar

File Name MAtch and Run

## Development

Requires CMake and a C11 compiler. A nix dev shell is available:
```
nix develop
```

Standard build
```
cmake -B build
cmake --build build
```

Useful development shell aliases
```
source dev_aliases.sh

# Reconfigure cmake
cfg

# Build and run
run
```

## Design Philosophy

Although this project is intended to produce a finished product that I will
actually use in my dev setup, I'm also using it as a platform to test out my
experimental xmacro-based `krs` C library.
