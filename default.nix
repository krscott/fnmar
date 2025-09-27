{
  cmake,
  lib,
  stdenv,
  prexy,
}:
stdenv.mkDerivation {
  name = "fnmar";
  src = lib.cleanSource ./.;

  nativeBuildInputs = [cmake prexy];

  configurePhase = ''
    cmake -B build
  '';

  buildPhase = ''
    cmake --build build
  '';

  installPhase = ''
    if [[ "$CC" == *"mingw32"* ]]; then
      # Workaround broken pkgCross cmake install
      mkdir -p "$out/bin"
      cp build/src/*.exe "$out/bin"
    else
      cmake --install build --prefix $out
    fi
  '';
}
