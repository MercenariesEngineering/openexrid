#!/usr/bin/env bash
#^^^ GCC documentation discourage the use of /bin/sh

# setup
set -x
set -e

mkdir -p "/tmp/gcc" && cd "/tmp/gcc"

# gcc
cd "/tmp/gcc"
wget -O - "https://ftp.fu-berlin.de/unix/languages/gcc/releases/gcc-11.2.0/gcc-11.2.0.tar.gz" | tar zxf -
cd gcc-11.2.0 && ./contrib/download_prerequisites && mkdir build && cd build
CC='gcc' CFLAGS='-m64 -O3' ../configure                                          \
    --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu   \
    --prefix=/usr/local/gcc-11.2.1 --enable-lto --enable-languages=c,c++           \
    --disable-multilib --disable-checking --enable-gcov
make -j `nproc`
make install

# clean
rm -rf "/tmp/gcc"

