#!/bin/bash

##############################################################################
## PARAMETERS

update_conan_packages="y"
update_cmake="y"
build_debug="y"
build_releasedebug="y"
build_release="y"

##############################################################################
## VALIDATION
set -e # exit when any command fails
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG # keep track of the last executed command
trap 'echo "\"${last_command}\" command filed with exit code $?."' EXIT # echo an error message before exiting

##############################################################################
## SCRIPT INPUT PARAMETERS

echo -e "\e[93m#################################\e[0m"
echo -e "\e[93m##                             ##\e[0m"
echo -e "\e[93m## Let's build OpenExr/Id !    ##\e[0m"
echo -e "\e[93m##                             ##\e[0m"
echo -e "\e[93m## Usage: $0 [options] ##\e[0m"
echo -e "\e[93m## Options :                   ##\e[0m"
echo -e "\e[93m## --skip_conan                ##\e[0m"
echo -e "\e[93m## --installer  (-> release)   ##\e[0m"
echo -e "\e[93m## --release    (no debug/dev) ##\e[0m"
echo -e "\e[93m## -h                          ##\e[0m"
echo -e "\e[93m##                             ##\e[0m"
echo -e "\e[93m#################################\e[0m"

while [[ $# -gt 0 ]]
do
key="$1"
case $key in
    -s|--skip_conan)
	update_conan_packages=""
    shift # past value
    ;;
    -i|--installer)
	build_installer="y"
	build_debug=""
	build_releasedebug=""
	build_release="y"
    shift # past value
    ;;
    -r|--release)
	build_debug=""
	build_releasedebug=""
	build_release="y"
    shift # past value
    ;;
    -h|--help)
    echo "Usage: $0 [--installer outputdir] [-h|--help]"
    echo "Options:"
    echo "  --installer: compile and generate installer"
    echo "  --skip_conan: Don't update dependencies"
    echo "  --release: only build release target, otherwise build release, relWithDebInfo, and debug"
    exit 0
    ;;
    *)
	# ignore unrecognized option
	shift
	;;
esac
done

##############################################################################
function updateConan()
{
	echo -e "\e[93m#################################\e[0m"
	echo -e "\e[93m## Updating Conan dependencies ##\e[0m"
	echo -e "\e[93m#################################\e[0m"

	#export CONAN_EXE="conan.exe"
	#if [ "$CONAN_HOME" ]; then
	#	export CONAN_USER_HOME="$CONAN_HOME"
	#fi

	export CONAN_EXE="$CONAN_1"
	if [ "$CONAN_1_HOME" ]; then
		export CONAN_USER_HOME="$CONAN_1_HOME"
	else
		export CONAN_USER_HOME="C:/Conan_1"
	fi

	echo "Using conan version:"
	"$CONAN_EXE" --version
	#"$CONAN_EXE" remote add pierousseau "https://api.bintray.com/conan/pierousseau/libs" --insert 0
	if [ "$build_debug" ]; then
		"$CONAN_EXE" install --update --build=outdated --build=cascade -g cmake_multi ./conanfile.py --install-folder ./$1/debug -s build_type=Debug -s compiler.version=$2
	fi
	if [ "$build_release" ]; then
		"$CONAN_EXE" install --update --build=outdated --build=cascade -g cmake_multi ./conanfile.py --install-folder ./$1/release -s build_type=Release -s compiler.version=$2
	fi

	if [ "$build_releasedebug" ]; then
		cp ./$1/release/conanbuildinfo_release.cmake ./$1/relwithdebinfo/conanbuildinfo_relwithdebinfo.cmake
		sed -i 's/_RELEASE/_RELWITHDEBINFO/g' ./$1/relwithdebinfo/conanbuildinfo_relwithdebinfo.cmake
	fi
}

##############################################################################
## WORK

if [ ! "$NUKE90_DIR" ]; then
	NUKE90_DIR="D:\\Nuke\\Nuke9.0v9"
fi
if [ ! "$NUKE100_DIR" ]; then
	NUKE105_DIR="D:\\Nuke\\Nuke10.0v6"
fi
if [ ! "$NUKE105_DIR" ]; then
	NUKE105_DIR="D:\\Nuke\\Nuke10.5v8"
fi
if [ ! "$NUKE111_DIR" ]; then
	NUKE111_DIR="D:\\Nuke\\Nuke11.1v6"
fi
if [ ! "$NUKE112_DIR" ]; then
	NUKE112_DIR="D:\\Nuke\\Nuke11.2v7"
fi
if [ ! "$NUKE113_DIR" ]; then
	NUKE113_DIR="D:\\Nuke\\Nuke11.3v6"
fi
if [ ! "$NUKE120_DIR" ]; then
	NUKE120_DIR="D:\\Nuke\\Nuke12.0v3"
fi
if [ ! "$NUKE121_DIR" ]; then
	NUKE121_DIR="D:\\Nuke\\Nuke12.1v2"
fi
if [ ! "$NUKE122_DIR" ]; then
	NUKE122_DIR="D:\\Nuke\\Nuke12.2v4"
fi

cd ..
mkdir -p build2010
mkdir -p build2015
if [ "$build_release" ]; then
	mkdir -p build2010/release
	mkdir -p build2015/release
fi
if [ "$build_debug" ]; then
	mkdir -p build2010/debug
	mkdir -p build2015/debug
fi
if [ "$build_releasedebug" ]; then
	mkdir -p build2010/relwithdebinfo
	mkdir -p build2015/relwithdebinfo
fi

##############################################################################
# Build with visual 2015

BUILD_FOLDER="build2015"
MSVC_NUM=14
MSVC_VER="Visual Studio 14 2015"
MSVC_ARCH="x64"

if [ "$update_conan_packages" ]; then
	updateConan $BUILD_FOLDER $MSVC_NUM
fi

echo -e "\e[93m#################################\e[0m"
echo -e "\e[93m## Generating 2015 Makefiles   ##\e[0m"
echo -e "\e[93m#################################\e[0m"

cd $BUILD_FOLDER/release
echo -e "\e[96mcmake -G \"$MSVC_VER\" -A $MSVC_ARCH ../../ -D USE_CONAN=1 -D BUILD_LIB=1 -D BUILD_PLUGINS=1 -D NUKE111_DIR=${NUKE111_DIR} -D NUKE112_DIR=${NUKE112_DIR} -D NUKE113_DIR=${NUKE113_DIR} -D NUKE120_DIR=${NUKE120_DIR} -D NUKE121_DIR=${NUKE121_DIR} -D NUKE122_DIR=${NUKE122_DIR} \e[0m"
cmake -G "$MSVC_VER" -A $MSVC_ARCH ../../ -D USE_CONAN=1 -D BUILD_LIB=1 -D BUILD_PLUGINS=1 -D NUKE111_DIR=${NUKE111_DIR} -D NUKE112_DIR=${NUKE112_DIR} -D NUKE113_DIR=${NUKE113_DIR} -D NUKE120_DIR=${NUKE120_DIR} -D NUKE121_DIR=${NUKE121_DIR} -D NUKE122_DIR=${NUKE122_DIR}
cd ../..

echo -e "\e[93m#################################\e[0m"
echo -e "\e[93m## Building 2015               ##\e[0m"
echo -e "\e[93m#################################\e[0m"

cd $BUILD_FOLDER/release
cmake --build . --target OpenEXRIdOFX --config Release
cmake --build . --target OpenEXRIdForNuke11.1 --config Release
cmake --build . --target OpenEXRIdForNuke11.2 --config Release
cmake --build . --target OpenEXRIdForNuke11.3 --config Release
cmake --build . --target OpenEXRIdForNuke12.0 --config Release
cmake --build . --target OpenEXRIdForNuke12.1 --config Release
cmake --build . --target OpenEXRIdForNuke12.2 --config Release
cd ../..


##############################################################################
# Build with visual 2010

BUILD_FOLDER="build2010"
MSVC_NUM=10
MSVC_VER="Visual Studio 10 2010"
MSVC_ARCH="x64"

if [ "$update_conan_packages" ]; then
	updateConan $BUILD_FOLDER $MSVC_NUM
fi

echo -e "\e[93m#################################\e[0m"
echo -e "\e[93m## Generating 2010 Makefiles   ##\e[0m"
echo -e "\e[93m#################################\e[0m"

cd $BUILD_FOLDER/release
echo -e "\e[96mcmake -G \"$MSVC_VER\" -A $MSVC_ARCH ../../ -D USE_CONAN=1 -D BUILD_LIB=1 -D BUILD_PLUGINS=1 -D NUKE90_DIR=${NUKE90_DIR} -D NUKE100_DIR=${NUKE100_DIR} -D NUKE105_DIR=${NUKE105_DIR}\e[0m"
cmake -G "$MSVC_VER" -A $MSVC_ARCH ../../ -D USE_CONAN=1 -D BUILD_LIB=1 -D BUILD_PLUGINS=1 -D NUKE90_DIR=${NUKE90_DIR} -D NUKE100_DIR=${NUKE100_DIR} -D NUKE105_DIR=${NUKE105_DIR}
cd ../..

echo -e "\e[93m#################################\e[0m"
echo -e "\e[93m## Building 2010               ##\e[0m"
echo -e "\e[93m#################################\e[0m"

cd $BUILD_FOLDER/release
cmake --build . --target OpenEXRIdForNuke9.0 --config Release
cmake --build . --target OpenEXRIdForNuke10.0 --config Release
cmake --build . --target OpenEXRIdForNuke10.5 --config Release
cd ../..

##############################################################################
# Installer

if [ "$build_installer" ]; then
	echo -e "\e[93m#################################\e[0m"
	echo -e "\e[93m## Building installer          ##\e[0m"
	echo -e "\e[93m#################################\e[0m"

	python make_installer.py

	echo -e "\e[93m#################################\e[0m"
	echo -e "\e[93m## Done !                      ##\e[0m"
	echo -e "\e[93m## Installer ready.            ##\e[0m"
	echo -e "\e[93m#################################\e[0m"
else
	echo -e "\e[93m#################################\e[0m"
	echo -e "\e[93m## Done !                      ##\e[0m"
	echo -e "\e[93m## Now you can open MSVC       ##\e[0m"
	echo -e "\e[93m## solution file in \"build\"    ##\e[0m"
	echo -e "\e[93m## folder.                     ##\e[0m"
	echo -e "\e[93m#################################\e[0m"
fi

exit 0
