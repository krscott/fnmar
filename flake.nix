{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    prexy.url = "github:krscott/prexy";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    prexy,
  }: let
    supportedSystems = [
      "x86_64-linux"
      "aarch64-linux"
      "x86_64-darwin"
      "aarch64-darwin"
    ];
  in
    flake-utils.lib.eachSystem supportedSystems (
      system: let
        pkgs = nixpkgs.legacyPackages.${system};

        # Final derivation including any overrides made to output package
        inherit (self.packages.${system}) fnmar;
      in {
        packages = {
          fnmar = pkgs.callPackage ./. {
            prexy = prexy.packages.${system}.prexy-stage1;
            stdenv = pkgs.clangStdenv;
          };

          fnmar-gcc = fnmar.override {
            inherit (pkgs) stdenv;
          };

          fnmar-win = fnmar.override {
            inherit (pkgs.pkgsCross.mingwW64) stdenv;
          };

          default = fnmar;
        };

        devShells = {
          default = pkgs.mkShell {
            inputsFrom = [fnmar];
            nativeBuildInputs = with pkgs; [
              shfmt
              alejandra
              clang-tools # NOTE: clang-tools must come before clang
              clang
            ];

            shellHook = ''
              source dev_shell.sh
            '';
          };
        };

        formatter = pkgs.alejandra;
      }
    );
}
