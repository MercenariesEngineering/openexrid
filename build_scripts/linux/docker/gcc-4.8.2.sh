#!/bin/sh
this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set -e

export build_dir=/tmp/gcc-4.8.2

# Download gcc 4.8.2
mkdir -p $build_dir
cd $build_dir

# build custom gcc-4.8.2
wget ftp://ftp.irisa.fr/pub/mirrors/gcc.gnu.org/gcc/releases/gcc-4.8.2/gcc-4.8.2.tar.gz
tar zxf gcc-4.8.2.tar.gz

wget ftp://ftp.irisa.fr/pub/mirrors/gcc.gnu.org/gcc/infrastructure/gmp-4.3.2.tar.bz2
tar jxf gmp-4.3.2.tar.bz2
mv gmp-4.3.2 gcc-4.8.2/gmp

wget ftp://ftp.irisa.fr/pub/mirrors/gcc.gnu.org/gcc/infrastructure/mpc-0.8.1.tar.gz
tar zxf mpc-0.8.1.tar.gz
mv mpc-0.8.1 gcc-4.8.2/mpc

wget ftp://ftp.irisa.fr/pub/mirrors/gcc.gnu.org/gcc/infrastructure/mpfr-2.4.2.tar.bz2
tar jxf mpfr-2.4.2.tar.bz2
mv mpfr-2.4.2 gcc-4.8.2/mpfr

export CC=gcc
export CXX=g++

# build custom gcc-4.8.2
mkdir -p $build_dir/build
cd $build_dir/build
../gcc-4.8.2/configure --prefix=/usr/local/gcc-4.8.2 --enable-languages=c,c++ --disable-multilib
make -j`nproc`
make install

# clean up
cd /
rm -rf $build_dir

ln -s /usr/local/gcc-4.8.2/bin/gcc /usr/bin/gcc-4.8
ln -s /usr/local/gcc-4.8.2/bin/g++ /usr/bin/g++-4.8
