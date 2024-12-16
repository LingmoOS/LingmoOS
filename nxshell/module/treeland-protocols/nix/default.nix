{ stdenv
, lib
, nix-filter
, cmake
, pkg-config
, wayland-scanner
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "treeland-protocols";
  version = "0.4";

  src = nix-filter.filter {
    root = ./..;

    exclude = [
      ".git"
      "debian"
      "LICENSES"
      "README.md"
      "README.zh_CN.md"
    ];
  };

  depsBuildBuild = [ pkg-config ];

  nativeBuildInputs = [
    cmake
    pkg-config
    wayland-scanner
  ];

  meta = {
    description = "Wayland protocol extensions for treeland";
    homepage = "https://github.com/lingmoos/treeland-protocols";
    license = with lib.licenses; [ gpl3Only lgpl3Only asl20 ];
    platforms = lib.platforms.linux;
    maintainers = with lib.maintainers; [ rewine ];
  };
})

