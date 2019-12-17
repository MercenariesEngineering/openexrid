#!/bin/bash

set -e

this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

cd ${this_directory}/../..
rm -rf build/release

echo "Installing conan dependencies ..."
conan remote add pierousseau https://api.bintray.com/conan/pierousseau/libs
conan install --update --build=outdated --build=cascade -g cmake ./conanfile.py --install-folder ./Conan/  -s build_type=Release

CC_VER=`conan profile get env.CC default`
CXX_VER=`conan profile get env.CXX default`

mkdir -p build/release
cd build/release

# CMake generate all makefiles
echo "Building makefiles ..."
cmake -G "Unix Makefiles" -D USE_CONAN=1 -D CONAN_BUILD_INFO_DIR=Conan -D CMAKE_BUILD_TYPE=Release -D CMAKE_C_COMPILER=$CC_VER -D CMAKE_CXX_COMPILER=$CXX_VER ../../

# And make
echo "Make ..."
make -j`nproc` VERBOSE=1

# And build the install package
cd ../..
python make_installer.py
