FROM debian:bookworm-slim

# Dependencies required to build GCC from source + MPI
RUN apt-get update && apt-get install -y \
    wget \
    build-essential \
    libgmp-dev \
    libmpc-dev \
    libmpfr-dev \
    libopenmpi-dev \
    openmpi-bin \
    && rm -rf /var/lib/apt/lists/*

# Build GCC 12.2.0 with plugin support enabled
# --disable-bootstrap speeds up the build significantly (~4x)
# --disable-multilib avoids needing 32-bit libs on the runner
RUN wget -q https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz \
    && tar xzf gcc-12.2.0.tar.gz \
    && mkdir gcc-12.2.0/build \
    && cd gcc-12.2.0/build \
    && ../configure \
        --prefix=/opt/gcc-12.2.0 \
        --enable-languages=c,c++ \
        --enable-plugin \
        --disable-bootstrap \
        --disable-multilib \
    && make -j$(nproc) \
    && make install \
    && cd / \
    && rm -rf gcc-12.2.0 gcc-12.2.0.tar.gz

ENV GCC_ROOT="/opt/gcc-12.2.0"
ENV PATH="/opt/gcc-12.2.0/bin:$PATH"
