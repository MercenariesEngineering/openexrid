#!/usr/bin/env bash
#^^^ GCC documentation discourage the use of /bin/sh

# setup
set -x
set -e

mkdir -p "${TOOLCHAIN_BUILD_DIR}/gcc" && cd "${TOOLCHAIN_BUILD_DIR}/gcc"

# gcc
cd "${TOOLCHAIN_BUILD_DIR}/gcc"
wget -O - "https://ftp.fu-berlin.de/unix/languages/gcc/releases/gcc-11.2.0/gcc-11.2.0.tar.gz" | tar zxf -
cd gcc-11.2.0 && ./contrib/download_prerequisites && mkdir build && cd build
CC='gcc' CFLAGS='-m64 -O3' ../configure                                          \
    --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu   \
    --prefix=${TOOLCHAIN_PREFIX} --enable-lto --enable-languages=c,c++           \
    --disable-multilib --disable-checking --enable-gcov
make -j `nproc`
make install

# clean
rm -rf "${TOOLCHAIN_BUILD_DIR}/gcc"

