# MPI Deadlock GCC Plugin

A GCC compiler plugin (v12.2.0) designed to detect potential deadlocks in MPI applications[cite: 1]. It analyzes the Control Flow Graph (CFG) to warn developers about invalid forks when evaluating MPI collectives.

## Prerequisites

* **GCC/G++:** Version 12.2.0[cite: 1].
* **MPI:** `mpicc` must be installed and in your PATH.
* **Graphviz:** Optional, used to generate `.dot` and `.png` visual representations of the CFGs.

## Setup

Before building, configure your environment variables:

1. Copy the template: `cp setenv.sh.template setenv.sh`
2. Edit `setenv.sh` to set your local `GCC_ROOT` path.
3. Source the file: `source setenv.sh`

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

Remove compiled objects and test binaries:

```bash
make clean

```

Remove all build artifacts, including `.dot` and `.png` files:

```bash
make cleanall

```



## License

This project is licensed under the [GNU General Public License v3.0](LICENSE).
