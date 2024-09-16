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
, pixman
, pam
, libxcrypt
}:

stdenv.mkDerivation (finalAttrs: {
  pname = "ddm";
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
    for file in $(grep -rl "/usr/bin")
    do
      substituteInPlace $file \
        --replace-fail "/usr/bin" "/run/current-system/sw/bin"
    done
  '';

  nativeBuildInputs = [
    cmake
    extra-cmake-modules
    pkg-config
    qttools
    wrapQtAppsHook
  ];

  buildInputs = [
    qtbase
    pixman
    pam
    libxcrypt
  ];

   cmakeFlags = [
    "-DCONFIG_FILE=/etc/ddm.conf"
    "-DCONFIG_DIR=/etc/ddm.conf.d"

    # Set UID_MIN and UID_MAX so that the build script won't try
    # to read them from /etc/login.defs (fails in chroot).
    # The values come from NixOS; they may not be appropriate
    # for running DDM outside NixOS, but that configuration is
    # not supported anyway.
    "-DUID_MIN=1000"
    "-DUID_MAX=29999"

    # we still want to run the DM on VT 7 for the time being, as 1-6 are
    # occupied by getties by default
    "-DSDDM_INITIAL_VT=7"

    "-DQT_IMPORTS_DIR=${placeholder "out"}/${qtbase.qtQmlPrefix}"
    "-DCMAKE_INSTALL_SYSCONFDIR=${placeholder "out"}/etc"
    "-DSYSTEMD_SYSTEM_UNIT_DIR=${placeholder "out"}/lib/systemd/system"
    "-DSYSTEMD_SYSUSERS_DIR=${placeholder "out"}/lib/sysusers.d"
    "-DSYSTEMD_TMPFILES_DIR=${placeholder "out"}/lib/tmpfiles.d"
    "-DDBUS_CONFIG_DIR=${placeholder "out"}/share/dbus-1/system.d"
  ];

  meta = {
    description = "DDM is a fork of SDDM";
    homepage = "https://github.com/linuxdeepin/ddm";
    license = with lib.licenses; [ gpl2Only ];
    platforms = lib.platforms.linux;
    maintainers = with lib.maintainers; [ rewine ];
  };
})

