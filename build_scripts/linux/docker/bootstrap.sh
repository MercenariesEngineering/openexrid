#!/bin/sh
this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set -e

# load keys to avoid annoying messages
rpm --import http://mirror.centos.org/centos/RPM-GPG-KEY-CentOS-6
rpm --import http://dl.fedoraproject.org/pub/epel/RPM-GPG-KEY-EPEL-6
# grant access to EPEL
yum -y install http://dl.fedoraproject.org/pub/epel/epel-release-latest-6.noarch.rpm
# make sure we do not use https for EPEL since it is not correctly handled           
sed -i 's/mirrorlist=https/mirrorlist=http/' /etc/yum.repos.d/epel.repo
# update trusted root certificates
yum -y update ca-certificates --disablerepo=epel
# install vim
yum -y install vim
# install SCL
yum -y install centos-release-scl
# install new linux headers 
yum -y install kernel-headers
# install new glibc (this is the first release that works for every dependences)
#rpm -Uvh $sources_dir/glibc-2.17-55.el6.x86_64.rpm $sources_dir/glibc-common-2.17-55.el6.x86_64.rpm $sources_dir/glibc-devel-2.17-55.el6.x86_64.rpm $sources_dir/glibc-headers-2.17-55.el6.x86_64.rpm &&
# install any required tool from repositories
yum -y install cmake3 bison flex tar bzip2 file wget patch nasm
# update SSL stuff
yum update -y nss curl libcurl

yum -y install gcc-c++.x86_64
g++ --version

# Download gcc 4.1.2 and 4.8.2
mkdir -p /build
cd /build

wget ftp://ftp.irisa.fr/pub/mirrors/gcc.gnu.org/gcc/releases/gcc-4.1.2/gcc-4.1.2.tar.gz
tar zxf gcc-4.1.2.tar.gz

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
mkdir -p /build/gcc-4.8.2-build
cd /build/gcc-4.8.2-build
../gcc-4.8.2/configure --prefix=/usr/local/gcc-4.8.2 --enable-languages=c,c++ --disable-multilib
make -j`nproc`
make install

# build custom gcc-4.1.2 (nuke9)
mkdir -p /build/gcc-4.1.2-build
cd /build/gcc-4.1.2-build
../gcc-4.1.2/configure --prefix=/usr/local/gcc-4.1.2 --enable-languages=c,c++ --disable-multilib
make -j`nproc`
make install

# clean up
cd /build
rm -rf gcc-4.8.2 gcc-4.1.2 gcc-4.8.2-build gcc-4.1.2-build
