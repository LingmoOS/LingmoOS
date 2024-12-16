{
  description = "A basic flake to help develop";

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

          treeland-protocols = pkgs.callPackage ./nix {
            nix-filter = nix-filter.lib;
          };
        in
        {
          packages = {
            default = treeland-protocols;     
          };
        }
      );
}
