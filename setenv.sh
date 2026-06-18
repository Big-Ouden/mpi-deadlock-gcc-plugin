#!/bin/bash

export GCC_ROOT="/path/to/gcc-12.2.0"

export CC="$GCC_ROOT/bin/gcc"
export CXX="$GCC_ROOT/bin/g++"

export PATH="$GCC_ROOT/bin:$PATH"

echo "Environment configured:"
echo "  CC=$CC"
echo "  CXX=$CXX"

