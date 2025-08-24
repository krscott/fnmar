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

        # Final derivation including any overrides made to output package
        finalDrv = self.packages.${system}.fnmar;
      in {
        packages = {
          fnmar = pkgs.callPackage ./. {};
          default = finalDrv;
        };

        # devShells = {
        #   default = pkgs.mkShell {
        #     inputsFrom = [finalDrv];
        #     nativeBuildInputs = [
        #       # add dev pkgs
        #     ];
        #   };
        # };

        formatter = pkgs.alejandra;
      }
    );
}
