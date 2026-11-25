# MPI Deadlock GCC Plugin

A GCC compiler plugin (v12.2.0) designed to detect potential deadlocks in MPI applications. It analyzes the Control Flow Graph (CFG) to warn developers about invalid forks when evaluating MPI collectives.

<img alt="GitHub Actions Workflow Status" src="https://img.shields.io/github/actions/workflow/status/Big-Ouden/mpi-deadlock-gcc-plugin/ci.yml?branch=main&style=for-the-badge">  <img alt="GitHub last commit" src="https://img.shields.io/github/last-commit/Big-Ouden/mpi-deadlock-gcc-plugin?style=for-the-badge">   
<img alt="GitHub repo size" src="https://img.shields.io/github/repo-size/Big-Ouden/mpi-deadlock-gcc-plugin?style=for-the-badge">  <img alt="GitHub License" src="https://img.shields.io/github/license/Big-Ouden/mpi-deadlock-gcc-plugin?style=for-the-badge">  

## Quick start with Nix (recommended)

The flake provides GCC 12.2.0, OpenMPI, Graphviz and make, so nothing else needs to be installed (requires Nix with flakes enabled):

```bash
nix develop        # shell with gcc 12.2.0, mpicc, dot and make
make && make check
```

`nix build` builds the plugin into `result/lib/` and runs the test suite. `nix flake check` does the same.

## Prerequisites

* **GCC/G++:** Version 12.2.0.
* **MPI:** `mpicc` must be installed and in your PATH.
* **Graphviz:** Optional, used to generate `.dot` and `.png` visual representations of the CFGs.

## File tree 

```bash
.
├── flake.lock
├── flake.nix
├── include
│   ├── graphviz.h
│   ├── MPI_collectives.def
│   ├── mpi_collectives.h
│   ├── plugin.h
│   └── plugin_headers.h
├── LICENSE
├── Makefile
├── report.pdf
├── README.md
├── slide.pdf
├── src
│   ├── graphviz.cpp
│   ├── mpi_collectives.cpp
│   └── plugin.cpp
└── tests
    ├── test-fail-1.c
    ├── test-fail-2.c
    ├── test-pass-1.c
    └── test-pass-2.c
```

## Build GCC 12.2.0

Skip this section if you use Nix. Otherwise, you need to build GCC 12.2.0 with plugins enabled to use this plugin. Before building GCC 12.2.0, some packages are necessary. If this is not done yet install *libgmp-dev*, *libmpc-dev* and *libmpfr-dev*. Once you have the archive from GNU, extract it. In the source folder, create a `MYBUILD` folder. Inside it, run the following command:
```bash
../configure --prefix=/home/<login>/GCC/gcc-12.2.0 --enable-languages=c,c++ --enable-plugin --disable-bootstrap --disable-multilib
```

Once the configuration stage is done, build GCC 12.2.0 and install it:
```bash
make -j4 && make install
```

## Compilation

Build the plugin (`build/libplugin.so`). Without Nix, point `CC`/`CXX` at your GCC 12.2.0 install and use the matching `mpicc` for the tests:

```bash
make CC=/path/to/gcc-12.2.0/bin/gcc CXX=/path/to/gcc-12.2.0/bin/g++
```

With Nix or with GCC 12.2.0 on your `PATH`, plain `make` is enough.

## Testing

`make check` compiles every test program in `tests/` against the plugin and verifies the result: `test-fail-*` must print a warning and `test-pass-*` must not. It exits non-zero on any failure.

```bash
make check MPICC=mpicc
```

To build a single test, run for example `make build/test-fail-1 MPICC=mpicc`. Test binaries are written to `build/`.

## Debug & Visualization

`make debug` rebuilds the plugin with the CFG dump enabled. Compiling a test then writes one Graphviz (`.dot`) file per function into `build/`:

```bash
make debug
make build/test-fail-1 MPICC=mpicc
```

Convert the generated `.dot` files into PNG format (saved in `build/png/`):

```bash
make dot2png

```

## Cleanup

Remove all build artifacts, including `.dot` and `.png` files:

```bash
make clean

```

## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).
