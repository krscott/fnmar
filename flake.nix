{
  inputs = {
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = {
    self,
    nixpkgs,
    flake-utils,
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

        useClang = true;

        # Final derivation including any overrides made to output package
        finalDrv = self.packages.${system}.fnmar;
      in {
        packages = {
          fnmar = pkgs.callPackage ./. {
            stdenv =
              if useClang
              then pkgs.clangStdenv
              else pkgs.stdenv;
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
              ${
                if useClang
                then "export CC=clang"
                else ""
              }

              source dev_aliases.sh
              debug
            '';
          };
        };

        formatter = pkgs.alejandra;
      }
    );
}
