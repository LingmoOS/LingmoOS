{
  description = "A basic flake to help develop treeland";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
    nix-filter.url = "github:numtide/nix-filter";
  };

  outputs = { self, nixpkgs, flake-utils, nix-filter }@input:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-linux" "riscv64-linux" ]
      (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};

          ddm = pkgs.qt6Packages.callPackage ./nix {
            nix-filter = nix-filter.lib;
          };
        in
        {
          packages = {
            default = ddm;
          };

          devShells.default = pkgs.mkShell {
            inputsFrom = [
              self.packages.${system}.default
            ];

            shellHook =
              let
                makeQtpluginPath = pkgs.lib.makeSearchPathOutput "out" pkgs.qt6.qtbase.qtPluginPrefix;
                makeQmlpluginPath = pkgs.lib.makeSearchPathOutput "out" pkgs.qt6.qtbase.qtQmlPrefix;
              in
              ''
                #export WAYLAND_DEBUG=1
                export QT_PLUGIN_PATH=${makeQtpluginPath (with pkgs.qt6; [ qtbase qtdeclarative qtquick3d qtimageformats qtwayland qt5compat qtsvg ])}
                export QML2_IMPORT_PATH=${makeQmlpluginPath (with pkgs.qt6; [ qtdeclarative qtquick3d qt5compat ]
                                                            ++ [ dde-nixos.packages.${system}.qt6.dtkdeclarative ] )}
                export QML_IMPORT_PATH=$QML2_IMPORT_PATH
              '';
          };
        }
      );
}
