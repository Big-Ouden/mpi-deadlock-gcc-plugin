{
  description = "MPI deadlock detection plugin for GCC 12.2.0";

  # GCC 12 has been removed from nixpkgs-unstable. The plugin API must match
  # GCC 12.2.0 exactly, so pin the last release that ships it (gcc12 = 12.2.0).
  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixos-23.05";

  outputs = { self, nixpkgs }:
    let
      systems = [ "x86_64-linux" "aarch64-linux" ];
      forAllSystems = f: nixpkgs.lib.genAttrs systems (system: f nixpkgs.legacyPackages.${system});
    in
    {
      # Build the plugin: nix build  ->  result/lib/libplugin.so
      packages = forAllSystems (pkgs: {
        default = pkgs.gcc12Stdenv.mkDerivation {
          pname = "mpi-deadlock-gcc-plugin";
          version = "0.1.0";
          src = self;

          # GMP/MPFR/MPC are needed because GCC's plugin headers include gmp.h etc.
          nativeBuildInputs = [ pkgs.gnumake pkgs.openmpi ];
          buildInputs = [ pkgs.gmp pkgs.mpfr pkgs.libmpc ];

          # Make mpicc (used by 'make check') use the same GCC as the plugin
          OMPI_CC = "gcc";
          OMPI_CXX = "g++";

          # The Makefile writes into build/, so install the .so from there
          buildPhase = ''
            runHook preBuild
            make CC=gcc CXX=g++
            runHook postBuild
          '';

          # Runs 'make check': the tests must warn or not warn as expected
          doCheck = true;
          checkPhase = ''
            runHook preCheck
            make check CC=gcc CXX=g++
            runHook postCheck
          '';

          installPhase = ''
            runHook preInstall
            mkdir -p $out/lib
            cp build/libplugin.so $out/lib/
            runHook postInstall
          '';
        };
      });

      # 'nix flake check' builds the package, which runs the test suite
      checks = forAllSystems (pkgs: {
        default = self.packages.${pkgs.system}.default;
      });

      # Development shell: GCC 12.2.0 with plugin support, MPI, Graphviz and make.
      #   nix develop
      #   make && make tests
      devShells = forAllSystems (pkgs: {
        default = (pkgs.mkShell.override { stdenv = pkgs.gcc12Stdenv; }) {
          packages = [ pkgs.gnumake pkgs.graphviz pkgs.openmpi pkgs.gmp pkgs.mpfr pkgs.libmpc ];

          # Make mpicc (used for the test programs) use the same GCC as the plugin
          OMPI_CC = "gcc";
          OMPI_CXX = "g++";

          shellHook = ''
            echo "gcc $(gcc -dumpversion), plugin headers: $(gcc -print-file-name=plugin)"
          '';
        };
      });
    };
}
