{
  description = "A basic flake to help develop treeland";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/master";
    flake-utils.url = "github:numtide/flake-utils";
    nix-filter.url = "github:numtide/nix-filter";
    waylib = {
      url = "github:vioken/waylib";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.flake-utils.follows = "flake-utils";
      inputs.nix-filter.follows = "nix-filter";
    };
    ddm = {
      url = "github:lingmoos/ddm";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.flake-utils.follows = "flake-utils";
      inputs.nix-filter.follows = "nix-filter";
    };
  };

  outputs = { self, nixpkgs, flake-utils, nix-filter, waylib, ddm }@input:
    flake-utils.lib.eachSystem [ "x86_64-linux" "aarch64-linux" "riscv64-linux" ]
      (system:
        let
          pkgs = nixpkgs.legacyPackages.${system};

          treeland = pkgs.qt6Packages.callPackage ./nix {
            nix-filter = nix-filter.lib;
            waylib = waylib.packages.${system}.default;
            ddm = ddm.packages.${system}.default;
          };
        in
        {
          nixosConfigurations.vm = nixpkgs.lib.nixosSystem {
            inherit system;
            modules = [
              {
                imports = [ "${nixpkgs}/nixos/modules/virtualisation/qemu-vm.nix" ];

                # TODO:  allow set Environment in services.seatd
                users.groups.seat = { };
                systemd.services.seatd = {
                  description = "Seat management daemon";
                  documentation = [ "man:seatd(1)" ];

                  wantedBy = [ "multi-user.target" ];
                  restartIfChanged = false;

                  serviceConfig = {
                    Type = "notify";
                    NotifyAccess = "all";
                    SyslogIdentifier = "seatd";
                    ExecStart = "${pkgs.sdnotify-wrapper}/bin/sdnotify-wrapper ${pkgs.seatd.bin}/bin/seatd -n 1 -u ocean -g ocean -l debug";
                    RestartSec = 1;
                    Restart = "always";
                    Environment = "SEATD_VTBOUND=0";
                  };
                };

                environment.systemPackages = with pkgs; [
                  foot
                  gdb
                  seatd
                  sdnotify-wrapper
                  treeland
                ];

                security.pam.services = {
                  ddm.text = ''
                    auth      substack      login
                    account   include       login
                    password  substack      login
                    session   include       login
                  '';

                  ddm-greeter.text = ''
                    auth     required       pam_succeed_if.so audit quiet_success user = ocean
                    auth     optional       pam_permit.so
                    account  required       pam_succeed_if.so audit quiet_success user = ocean
                    account  sufficient     pam_unix.so
                    password required       pam_deny.so
                    session  required       pam_succeed_if.so audit quiet_success user = ocean
                    session  required       pam_env.so conffile=/etc/pam/environment readenv=0
                    session  optional       ${pkgs.systemd}/lib/security/pam_systemd.so
                    session  optional       pam_keyinit.so force revoke
                    session  optional       pam_permit.so
                  '';

                  ddm-autologin.text = ''
                    auth     requisite pam_nologin.so
                    auth     required  pam_succeed_if.so uid >= 1000 quiet
                    auth     required  pam_permit.so
                    account  include   system-local-login
                    password include   system-local-login
                    session  include   system-local-login
                  '';
                };

                # services.xserver.displayManager.defaultSession =  "TreeLand";
                # services.xserver.displayManager.sessionPackages = [ treeland ];
                environment.etc."ddm.conf".text = ''
                  [General]
                  DisplayServer=single
                  HaltCommand=/run/current-system/systemd/bin/systemctl poweroff
                  Numlock=none
                  RebootCommand=/run/current-system/systemd/bin/systemctl reboot
                  [Users]
                  HideShells=/run/current-system/sw/bin/nologin
                  HideUsers=nixbld1,nixbld10,nixbld11,nixbld12,nixbld13,nixbld14,nixbld15,nixbld16,nixbld17,nixbld18,nixbld19,nixbld2,nixbld20,nixbld21,nixbld22,nixbld23,nixbld24,nixbld25,nixbld26,nixbld27,nixbld28,nixbld29,nixbld3,nixbld30,nixbld31,nixbld32,nixbld4,nixbld5,nixbld6,nixbld7,nixbld8,nixbld9
                  MaximumUid=30000
                  [Wayland]
                  SessionDir=${treeland}/share/wayland-sessions
                  [Single]
                  CompositorCommand=treeland
                  SessionDir=${treeland}/share/wayland-sessions
                '';
                environment.pathsToLink = [ "/share/ddm" ];

                users.groups.ocean = { };
                users.users.ocean = {
                  home = "/var/lib/ddm";
                  group = "ocean";
                  isSystemUser = true;
                };
                services.lingmo.ocean-daemon.enable = true; # DDM

                services.dbus.packages = [ treeland ];
                systemd.tmpfiles.packages = [ treeland ];
                # systemd.packages = [ treeland ];
                systemd.services.display-manager.after = [
                  "systemd-user-sessions.service"
                  "getty@tty7.service"
                  "plymouth-quit.service"
                  "systemd-logind.service"
                ];
                systemd.services.display-manager.conflicts = [
                  "getty@tty7.service"
                ];

                systemd.services.display-manager.enable = true;

                services.xserver.enable = true;
                services.xserver.displayManager.job = {
                  execCmd = "exec /run/current-system/sw/bin/ddm";
                };
                # To enable user switching, allow sddm to allocate TTYs/displays dynamically.
                services.xserver.tty = null;
                services.xserver.display = null;

                services.accounts-daemon.enable = true;

                hardware.opengl.enable = true;
                programs.xwayland.enable = true;
                security.polkit.enable = true;

                services.dbus.enable = true;
                services.openssh.enable = true;
                services.xserver.displayManager.lightdm.enable = false;

                users.users.test = {
                  isNormalUser = true;
                  uid = 1000;
                  extraGroups = [ "wheel" "networkmanager" "sound" "video" "input" "tty" "ssh" ];
                  password = "test";
                };

                users.users.test2 = {
                  isNormalUser = true;
                  uid = 1001;
                  password = "test2";
                };

                virtualisation = {
                  qemu.options = [ "-vga virtio -device virtio-gpu-pci" ];
                  cores = 8;
                  memorySize = 8192;
                  diskSize = 16384;
                  resolution = { x = 1920; y = 1080; };
                };
                system.stateVersion = "24.05";
              }
            ];
          };

          packages = {
            default = treeland;
            qemu = self.nixosConfigurations.${system}.vm.config.system.build.vm;
          };

          apps.qemu = {
            type = "app";
            program = "${self.packages.${system}.qemu}/bin/run-nixos-vm";
          };

          devShells.default = pkgs.mkShell {
            packages = with pkgs; [
              # For submodule build
              libinput
              wayland
            ];

            inputsFrom = [
              self.packages.${system}.default
            ];

            shellHook =
              let
                makeQtpluginPath = pkgs.lib.makeSearchPathOutput "out" pkgs.qt6.qtbase.qtPluginPrefix;
                makeQmlpluginPath = pkgs.lib.makeSearchPathOutput "out" pkgs.qt6.qtbase.qtQmlPrefix;
              in
              ''
                #export QT_LOGGING_RULES="*.debug=true;qt.*.debug=false"
                #export WAYLAND_DEBUG=1
                export QT_PLUGIN_PATH=${makeQtpluginPath (with pkgs.qt6; [ qtbase qtdeclarative qtquick3d qtimageformats qtwayland qt5compat qtsvg ])}
                export QML2_IMPORT_PATH=${makeQmlpluginPath (with pkgs; with qt6; [ qtdeclarative qtquick3d qt5compat lingmo.dtk6declarative ] )}
                export QML_IMPORT_PATH=$QML2_IMPORT_PATH
              '';
          };
        }
      );
}
