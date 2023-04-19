#!/bin/sh
this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set -e

# Download cmake 3.16.8 binaries
cd /tmp
wget https://github.com/Kitware/CMake/releases/download/v3.16.8/cmake-3.16.8-Linux-x86_64.tar.gz
tar zxf cmake-3.16.8-Linux-x86_64.tar.gz
mv cmake-3.16.8-Linux-x86_64 /usr/local/cmake-3.16.8
rm -rf cmake-3.16.8-Linux-x86_64.tar.gz
