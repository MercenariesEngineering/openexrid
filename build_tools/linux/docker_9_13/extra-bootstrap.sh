#!/bin/sh
this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set -e

ln -s /usr/local/gcc-4.1.2/bin/gcc /usr/bin/gcc-4.1
ln -s /usr/local/gcc-4.1.2/bin/g++ /usr/bin/g++-4.1
ln -s /usr/local/gcc-4.8.2/bin/gcc /usr/bin/gcc-4.8
ln -s /usr/local/gcc-4.8.2/bin/g++ /usr/bin/g++-4.8

yum -y install mesa-libGL-devel mesa-libGLU-devel rh-python35 sclo-git212

# grab a fresh version of pip for python 3.5 
scl enable rh-python35 "python $this_directory/get-pip.py"
scl enable rh-python35 "python -m pip install --upgrade setuptools"

# Download patch 2.7.6 and compile
cd /tmp
wget http://ftp.gnu.org/gnu/patch/patch-2.7.6.tar.gz
tar zxf patch-2.7.6.tar.gz
rm -rf patch-2.7.6.tar.gz
mkdir build
ls -la /tmp/patch-2.7.6
/tmp/patch-2.7.6/configure --prefix=/usr/local/patch-2.7.6
make
make install
cd /tmp
rm -rf build patch-2.7.6

# Download binutils 2.30 and compile
cd /tmp
wget http://ftp.gnu.org/gnu/binutils/binutils-2.30.tar.gz
tar zxf binutils-2.30.tar.gz
rm -rf binutils-2.30.tar.gz
mkdir build
cd build
/tmp/binutils-2.30/configure --prefix=/usr/local/binutils-2.30
make
make install
cd /tmp
rm -rf build binutils-2.30

