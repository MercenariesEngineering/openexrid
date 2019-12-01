#!/bin/sh
this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set -e

export build_dir=/tmp/gcc-4.8.2

# Download gcc 4.1.2
mkdir -p $build_dir
cd $build_dir

wget ftp://ftp.irisa.fr/pub/mirrors/gcc.gnu.org/gcc/releases/gcc-4.1.2/gcc-4.1.2.tar.gz
tar zxf gcc-4.1.2.tar.gz

export CC=gcc
export CXX=g++

# build custom gcc-4.1.2 (nuke9)
mkdir -p $build_dir/build
cd $build_dir/build
../gcc-4.1.2/configure --prefix=/usr/local/gcc-4.1.2 --enable-languages=c,c++ --disable-multilib
make -j`nproc`
make install

# clean up
cd /
rm -rf $build_dir

ln -s /usr/local/gcc-4.1.2/bin/gcc /usr/bin/gcc-4.1
ln -s /usr/local/gcc-4.1.2/bin/g++ /usr/bin/g++-4.1
