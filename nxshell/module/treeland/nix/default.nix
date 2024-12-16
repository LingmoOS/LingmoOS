{ stdenv
, lib
, nix-filter
, cmake
, extra-cmake-modules
, pkg-config
, wayland-scanner
, qttools
, wrapQtAppsHook
, qtbase
, qtquick3d
, qtimageformats
, qtwayland
, qtsvg
, ddm
, lingmo
, waylib
, wayland
, wayland-protocols
, wlr-protocols
, pixman
, pam
, libxcrypt
, nixos-artwork
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "treeland";
  version = "0.2.2";

  src = nix-filter.filter {
    root = ./..;

    exclude = [
      ".git"
      "debian"
      "LICENSES"
      "README.md"
      "README.zh_CN.md"
      (nix-filter.matchExt "nix")
    ];
  };

  postPatch = ''
    for file in $(grep -rl "/usr/share/wallpapers/lingmo/desktop.jpg")
    do
      substituteInPlace $file \
        --replace-fail "/usr/share/wallpapers/lingmo/desktop.jpg" \
                "${nixos-artwork.wallpapers.simple-blue}/share/backgrounds/nixos/nix-wallpaper-simple-blue.png"
    done
  '';

  nativeBuildInputs = [
    cmake
    extra-cmake-modules
    pkg-config
    wayland-scanner
    qttools
    wrapQtAppsHook
  ];

  buildInputs = [
    qtbase
    qtquick3d
    qtimageformats
    qtwayland
    qtsvg
    ddm
    lingmo.dtk6declarative
    lingmo.dtk6systemsettings
    waylib
    wayland
    wayland-protocols
    wlr-protocols
    pixman
    pam
    libxcrypt
  ];

   cmakeFlags = [
    "-DQT_IMPORTS_DIR=${placeholder "out"}/${qtbase.qtQmlPrefix}"
    "-DCMAKE_INSTALL_SYSCONFDIR=${placeholder "out"}/etc"
    "-DSYSTEMD_SYSTEM_UNIT_DIR=${placeholder "out"}/lib/systemd/system"
    "-DSYSTEMD_SYSUSERS_DIR=${placeholder "out"}/lib/sysusers.d"
    "-DSYSTEMD_TMPFILES_DIR=${placeholder "out"}/lib/tmpfiles.d"
    "-DDBUS_CONFIG_DIR=${placeholder "out"}/share/dbus-1/system.d"
  ];

  meta = {
    description = "DDM is a fork of SDDM";
    homepage = "https://github.com/vioken/treeland";
    license = with lib.licenses; [ gpl3Only lgpl3Only asl20 ];
    platforms = lib.platforms.linux;
    maintainers = with lib.maintainers; [ rewine ];
  };
})

