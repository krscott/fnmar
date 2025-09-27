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
source dev_shell.sh

# Reconfigure cmake
cfg

# Build and run
run

# Setup vscode debugging
setup_vscode
```

### Debugging

Example [`launch.json`](dev/vscode/launch.json) and 
[`tasks.json`](dev/vscode/tasks.json) files are included to debug in vscode.

Nix home-manager setup
```nix
    programs.vscode = {
      enable = true;
      package = pkgs.vscodium;
      profiles.default.extensions = with pkgs.vscode-extensions; [
        llvm-vs-code-extensions.lldb-dap
        llvm-vs-code-extensions.vscode-clangd
      ];
      userSettings = lib.importJSON ./settings.json;
      keybindings = lib.importJSON ./keybindings.json;
    };
```

## Design Philosophy

Although this project is intended to produce a finished product that I will
actually use in my dev setup, I'm also using it as a platform to test out my
experimental xmacro-based `krs` C library.
