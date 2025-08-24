{
  cmake,
  lib,
  stdenv,
}:
stdenv.mkDerivation {
  name = "fnmar";
  src = lib.cleanSource ./.;

  nativeBuildInputs = [cmake];

  configurePhase = ''
    cmake -B build
  '';

  buildPhase = ''
    cmake --build build
  '';

  installPhase = ''
    cmake --install build --prefix $out
  '';
}
