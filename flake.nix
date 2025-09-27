{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
    prexy-flake.url = "github:krscott/prexy";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
    prexy-flake,
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
        prexy = prexy-flake.packages.${system}.prexy-stage1;

        # Final derivation including any overrides made to output package
        finalDrv = self.packages.${system}.fnmar;
      in {
        packages = {
          fnmar = pkgs.callPackage ./. {
            inherit prexy;
            stdenv = pkgs.clangStdenv;
          };

          fnmar-gcc = finalDrv.override {
            inherit (pkgs) stdenv;
          };

          fnmar-win = finalDrv.override {
            inherit (pkgs.pkgsCross.mingwW64) stdenv;
          };

          default = finalDrv;
        };

        devShells = {
          default = pkgs.mkShell {
            inputsFrom = [finalDrv];
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
