# MPI Deadlock GCC Plugin

A GCC compiler plugin (v12.2.0) designed to detect potential deadlocks in MPI applications[cite: 1]. It analyzes the Control Flow Graph (CFG) to warn developers about invalid forks when evaluating MPI collectives.

## Prerequisites

* **GCC/G++:** Version 12.2.0[cite: 1].
* **MPI:** `mpicc` must be installed and in your PATH.
* **Graphviz:** Optional, used to generate `.dot` and `.png` visual representations of the CFGs.

## File tree 

```bash
.
├── include
│   ├── graphviz.h
│   ├── MPI_collectives.def
│   ├── mpi_collectives.h
│   ├── plugin.h
│   └── plugin_headers.h
├── LICENSE
├── Makefile
├── report.pdf
├── README.md
├── setenv.sh
├── slide.pdf
├── src
│   ├── graphviz.cpp
│   ├── mpi_collectives.cpp
│   └── plugin.cpp
└── tests
    ├── test-fail-1.c
    ├── test-fail-2.c
    ├── test-pass-1.c
    └── test-pass-2.c
```



## Build GCC12 

You need to build GCC 12.2.0 with plugins enabled to use this plugin. Before building GCC 12.2.0, some packages are necessary. If this is not done yet install *libgmp-dev*, *libmpc-dev* and *libmpfr-dev*. Once you got archive from the compiler, extract it. In source file folder, create a MYBUILD folder. Inside run following command : 
```bash
../configure --prefix=/home/<login>/GCC/gcc-10.2.0 --enable-languages=c,c++,fortran --enable-plugin --disable-bootstrap --disable-multilib
```


One the configuration stage done you need to to build GCC 12.2.0 and install it running :
```bash
make _j4 && make install
```

## Setup

Before building, configure your environment variables:

1. Edit `setenv.sh` to set your local `GCC_ROOT` path.
2. Source the file: `source setenv.sh`

## Compilation

Build the plugin (`libplugin.so`) with the following command[cite: 1]:

```bash
make

```

## Testing

Run tests to verify the plugin's behavior. The compiled test binaries will be generated in the root directory.

```bash
# Compile specific tests
make test-fail-1  # Should trigger warnings[cite: 1]
make test-pass-1  # Should compile without warnings[cite: 1]

# Compile all available tests from the tests/ directory
make tests

```

## Debug & Visualization

Generate Graphviz (`.dot`) representations of the CFG:

```bash
make debug

```

Convert the generated `.dot` files into PNG format (saved in the `png/` directory):

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
