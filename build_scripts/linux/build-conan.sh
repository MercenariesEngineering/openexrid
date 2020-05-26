#!/bin/bash

set -e

THIS_DIRECTORY="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_ROOT=${THIS_DIRECTORY}/../..
cd ${BUILD_ROOT}

conan remote add pierousseau https://api.bintray.com/conan/pierousseau/libs

function prebuild()
{
	CONAN_PROFILE=$1

	GCC_VERSION=`conan profile get settings.compiler.version $CONAN_PROFILE`
	BUILD_FOLDER=${BUILD_ROOT}/build${GCC_VERSION}
	CC_VER=`conan profile get env.CC $CONAN_PROFILE`
	CXX_VER=`conan profile get env.CXX $CONAN_PROFILE`
	echo -e "\e[93m#################################\e[0m"
	echo -e "\e[93m## Dependencies for gcc ${GCC_VERSION}    ##\e[0m"
	echo -e "\e[93m#################################\e[0m"

	cd ${BUILD_ROOT}
	conan install --update --build=outdated --build=cascade -g cmake -pr $CONAN_PROFILE ./conanfile.py --install-folder ./Conan/ -s build_type=Release

	rm -rf $BUILD_FOLDER
	mkdir -p $BUILD_FOLDER
	cd $BUILD_FOLDER

	# CMake generate all makefiles
	echo -e "\e[93m#################################\e[0m"
	echo -e "\e[93m## Running CMake for gcc ${GCC_VERSION}   ##\e[0m"
	echo -e "\e[93m#################################\e[0m"
	cmake -G "Unix Makefiles" -D USE_CONAN=1 -D CONAN_BUILD_INFO_DIR=Conan -D CMAKE_BUILD_TYPE=Release -D CMAKE_C_COMPILER=$CC_VER -D CMAKE_CXX_COMPILER=$CXX_VER ../

	echo -e "\e[93m#################################\e[0m"
	echo -e "\e[93m## Building with gcc ${GCC_VERSION}       ##\e[0m"
	echo -e "\e[93m#################################\e[0m"
}

# Build libraries relying on gcc-4.8
prebuild ${BUILD_ROOT}/build_scripts/linux/docker/conan_profile_linux
make -j`nproc` OpenEXRIdOFX
make -j`nproc` OpenEXRIdForNuke10.5
make -j`nproc` OpenEXRIdForNuke11.1
make -j`nproc` OpenEXRIdForNuke11.2
make -j`nproc` OpenEXRIdForNuke11.3
make -j`nproc` OpenEXRIdForNuke12.0
make -j`nproc` OpenEXRIdForNuke12.1

# Build libraries relying on gcc-4.1
prebuild ${BUILD_ROOT}/build_scripts/linux/docker/conan_profile_linux_gcc4.1
make -j`nproc` OpenEXRIdOFX
#make -j`nproc` OpenEXRIdForNuke9.0

echo -e "\e[93m#################################\e[0m"
echo -e "\e[93m## Packaging                   ##\e[0m"
echo -e "\e[93m#################################\e[0m"
cd $BUILD_ROOT
python make_installer.py
