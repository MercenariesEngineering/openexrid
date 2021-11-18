#!/bin/bash

##############################################################################
## ENVIRONMENT SETUP

root_directory="$(readlink -f "$(dirname "$0")")"

# File .build_env can contain specific environment variables, e.g. add Python3 to path, configure conan... But don't run it from docker
if [ ! -f /.dockerenv ]; then
	if [ ! -f .build_env ]; then
		echo -e "\e[96m#################################\e[0m"
		echo -e "\e[96m## Generating an empty .build_env file. Use it to define helpful env variables (see .build_env.example) ##\e[0m"
		echo -e "\e[96m#################################\e[0m"
		touch ${root_directory}/.build_env
	fi
	source ${root_directory}/.build_env
fi

##############################################################################
## CALLING OS ?

uname_out="$(uname -s)"
case "${uname_out}" in
	Linux*)     machine=Linux;;
	Darwin*)    machine=Mac;;
	CYGWIN*)    machine=Windows;;
	MINGW*)     machine=Windows;;
	*)          machine="UNKNOWN:${uname_out}"
esac

##############################################################################
## VALIDATION

set -e # exit when any command fails
trap 'last_command=$current_command; current_command=$BASH_COMMAND' DEBUG # keep track of the last executed command
#trap 'echo "\"${last_command}\" command filed with exit code $?." && read -p "press any key to continue" && exit 1' EXIT # echo an error message before exiting
trap 'echo "\"${last_command}\" command filed with exit code $?."' EXIT # echo an error message before exiting

##############################################################################
## PARAMETERS
build_root_directory="${root_directory}" 
build_conan_release="y"
build_conan_relwithdebinfo=""
build_conan_debug=""
build_installer=""

# The following parameters may be defined in .build_env, use the following defaults, or be overridden from command-line

# Should we update conan ? ("full"/"fast") 
if [ ! "$conan_update" ]; then
	conan_update="full"
fi
# default_build_type
if [ ! "$build_type" ]; then
	build_type="Release"
fi

##############################################################################
## SCRIPT INPUT PARAMETERS

echo -e "\e[93m########################################################\e[0m"
echo -e "\e[93m##                                                    ##\e[0m"
echo -e "\e[93m## Let's build OpenExr/Id !                           ##\e[0m"
echo -e "\e[93m##                                                    ##\e[0m"
echo -e "\e[93m## Usage: $0 [options]\e[0m"
echo -e "\e[93m## Options :                                          ##\e[0m"
echo -e "\e[93m## --installer                                        ##\e[0m"
echo -e "\e[93m## --conan-update [full|fast]                         ##\e[0m"
echo -e "\e[93m## --build [config]                                   ##\e[0m"
echo -e "\e[93m## -h                                                 ##\e[0m"
echo -e "\e[93m##                                                    ##\e[0m"
echo -e "\e[93m########################################################\e[0m"

while [[ $# -gt 0 ]]
do
key="$1"
case $key in
	--installer)
	build_installer="y"
	shift
	;;

	--build)
	build="y"
	shift # past value
	if [[ $# -gt 0 ]]; then
		case $1 in
			dev|Dev|RelWithDebInfo)
			build_type="RelWithDebInfo"
			shift
			;;
			release|Release)
			build_type="Release"
			shift
			;;
			debug|Debug)
			build_type="Debug"
			shift
			;;
			*)
			build_type="Release"
			;;
		esac
	fi
	;; 

	--conan-update)
	conan_update="full"
	shift # past value
	if [[ $# -gt 0 ]]; then
		case $1 in
			full|fast)
			conan_update=$1
			shift
			;;
		esac
	fi
	;;

	-h|--help)
	echo "Usage: $0 [--installer] [--build [config]] [--conan-update [full|fast|docker|docker_fast]] [-h|--help]"
	echo "Options:"
	echo "  --installer: compile and generate installer"
	echo "  --conan-update [full|fast]: update conan packages assuming locally out-of-date or up-to-date"
	echo "  --build: compile. Optional parameter: [release|dev|debug] (default: release)"
	exit 0
	;;
	*) 
	echo -e "\e[101m\nUnrecognized option: $key\n\e[0m"
	shift # past value
	;;
esac
done

##############################################################################
 
build_directory="${build_root_directory}/build"
mkdir -p "${build_directory}"

##############################################################################

if [ "$build_installer" ]; then
	build="y"
	build_type="Release"
fi

##############################################################################

function ConanSetUpdateStrategy()
{
	if [ "$conan_update" == "fast" ] ; then
		conan_update_strategy="--build=outdated --build=cascade"
	else
		conan_update_strategy="--update --build=outdated --build=cascade"
	fi
}

function ConanInstall()
{
	local config=$1
	local outfile=$2
	local profile=$3
	echo "conan install ${root_directory}/conanfile.py $conan_update_strategy $conan_generator -pr $profile -s build_type=$config -o build_plugins=True -if $outdir"
	conan install ${root_directory}/conanfile.py $conan_update_strategy $conan_generator -pr $profile -s build_type=$config -o build_plugins=True -if $outdir
}

function ConanBuild()
{
	local config=$1
	local outfile=$2
	local outdir="$(dirname $outfile)"
	local profile=$3
	ConanSetUpdateStrategy
	echo -e "\n\e[96m## Conan: Build $config\e[0m\n"
	mkdir -p $outdir 
	ConanInstall ${config} ${outdir} ${profile}

	if [ "$machine" == "Windows" ]; then
		# virtualenv activation scripts would be overwritten by the next built config
		venv_dst=$outdir/$config
		mkdir -p $venv_dst
		mv $outdir/activate.bat $outdir/deactivate.bat $outdir/environment.bat.env $outdir/activate.ps1 $outdir/deactivate.ps1 $outdir/environment.ps1.env $outdir/activate.sh $outdir/deactivate.sh $outdir/environment.sh.env $outdir/$config
		# Update scripts for altered paths
		sed -i "s/build\\\\Conan/build\\\\Conan\\\\$config/g" $venv_dst/activate.bat
		sed -i "s/build\\\\Conan/build\\\\Conan\\\\$config/g" $venv_dst/activate.ps1
		sed -i "s/build\\\\Conan/build\\\\Conan\\\\$config/g" $venv_dst/activate.sh
		# Fix bash script using C:\path instead of /C/path
		sed -i 's/\(.\):\\/\/\1\//g' $venv_dst/environment.sh.env
		sed -i 's/\\/\//g' $venv_dst/environment.sh.env
	fi
}

function ConanDuplicateBuild()
{
	local config_dst=$1
	local outfile_dst=$2
	local outdir_dst="$(dirname $outfile_dst)"
	local suffix_dst=$3
	local config_src=$4
	local outfile_src=$5
	local outdir_src="$(dirname $outfile_src)"
	local suffix_src=$6

	echo -e "\n\e[36m## Conan: Copy $config_src as $config_dst\e[0m"
	mkdir -p $outdir_dst
	# copy and patch cmake file
	cp $outfile_src $outfile_dst
	sed -i "s/_$suffix_src/_$suffix_dst/g" $outfile_dst
	#copy virtualenv files
	if [ "$machine" == "Windows" ]; then
		venv_src=$outdir_src/$config_src
		venv_dst=$outdir_dst/$config_dst
		mkdir -p $venv_dst
		cp $venv_src/activate.bat $venv_src/deactivate.bat $venv_src/environment.bat.env $venv_src/activate.ps1 $venv_src/deactivate.ps1 $venv_src/environment.ps1.env $venv_src/activate.sh $venv_src/deactivate.sh $venv_src/environment.sh.env $venv_dst
		for f in activate.bat deactivate.bat activate.ps1 deactivate.ps1 activate.sh deactivate.sh ; do
			sed -i "s/$config_src/$config_dst/g" $venv_dst/$f
		done
	fi
}

function ConanBuildOrDuplicateBuild()
{
	local do_build=$1
	local conan_file=$2
	local config=$3
	local suffix=$4
	local profile=$5
	if [ "$do_build" ] || ( [ "$machine" == "Windows" ] && [ "$config" == "Release" ] ) ; then
		ConanBuild $config $conan_file $profile
	else
		if [ "$machine" == "Windows" ]; then
			ConanDuplicateBuild $config $conan_file $suffix Release $conan_release_file RELEASE
		fi
	fi
}

function ConanUpdate()
{
	local profile=$1
	local compiler="$(conan profile get settings.compiler $profile)"
	local compiler_version="$(conan profile get settings.compiler.version $profile)"

	echo -e "\e[93m#################################\e[0m"
	echo -e "\e[93m## Updating Conan dependencies ##\e[0m"
	echo -e "\e[93m## $compiler $compiler_version\e[0m"
	echo -e "\e[93m#################################\e[0m"

	echo "Using conan version:"
	conan --version 
	if [ -d "$CONAN_USER_HOME" ]; then
		echo "In conan folder: $CONAN_USER_HOME"
	else 
		export CONAN_USER_HOME=
		echo "In conan default folder"
	fi
	echo "Conan profile: $profile"
	echo "Conan remotes:"
	conan remote list

	if [ "$machine" == "Windows" ]; then
		conan_generator="-g cmake_multi -g virtualenv"
		conan_release_file="${build_directory}/Conan/conanbuildinfo_release.cmake"
		conan_relwithdebinfo_file="${build_directory}/Conan/conanbuildinfo_relwithdebinfo.cmake"
		conan_debug_file="${build_directory}/Conan/conanbuildinfo_debug.cmake"
	elif [ "$machine" == "Linux" ]; then
		conan_generator="-g cmake"
		conan_release_file="${build_directory}/Conan/conanbuildinfo.cmake"
		conan_relwithdebinfo_file="${build_directory}/Conan/conanbuildinfo.cmake"
		conan_debug_file="${build_directory}/Conan/conanbuildinfo.cmake"
	fi

	ConanBuildOrDuplicateBuild "${build_conan_release}" $conan_release_file Release RELEASE $profile
	ConanBuildOrDuplicateBuild "${build_conan_relwithdebinfo}" $conan_relwithdebinfo_file RelWithDebInfo RELWITHDEBINFO $profile
	ConanBuildOrDuplicateBuild "${build_conan_debug}" $conan_debug_file Debug DEBUG $profile
}

##############################################################################
## WORK

conan_actions="--configure"
if [ "$build" ]; then
	conan_actions="$conan_actions --build"
fi

if [ "$machine" == "Windows" ]; then
	mkdir -p ${build_directory}/build2010
	mkdir -p ${build_directory}/build2015

	ConanUpdate ${root_directory}/build_tools/conan_profile_windows_vs2015
	CMAKE_BUILD_TYPE=$config conan build --install-folder ${build_directory}/Conan/ --source-folder ${root_directory} --build-folder ${build_directory}/build2015 --package-folder ${build_root_directory}/bin ${conan_actions} ${root_directory}/conanfile.py

	ConanUpdate ${root_directory}/build_tools/conan_profile_windows_vs2010
	CMAKE_BUILD_TYPE=$config conan build --install-folder ${build_directory}/Conan/ --source-folder ${root_directory} --build-folder ${build_directory}/build2010 --package-folder ${build_root_directory}/bin ${conan_actions} ${root_directory}/conanfile.py
else
	mkdir -p ${build_directory}/buildgcc41
	mkdir -p ${build_directory}/buildgcc

	ConanUpdate ${root_directory}/build_tools/conan_profile_linux
	CMAKE_BUILD_TYPE=$config conan build --install-folder ${build_directory}/Conan/ --source-folder ${root_directory} --build-folder ${build_directory}/buildgcc --package-folder ${build_root_directory}/bin ${conan_actions} ${root_directory}/conanfile.py

	ConanUpdate ${root_directory}/build_tools/conan_profile_linux_gcc4.1
	CMAKE_BUILD_TYPE=$config conan build --install-folder ${build_directory}/Conan/ --source-folder ${root_directory} --build-folder ${build_directory}/buildgcc41 --package-folder ${build_root_directory}/bin ${conan_actions} ${root_directory}/conanfile.py
fi


if [ "$build_installer" ] ; then
	echo -e "\e[93m#################################\e[0m"
	echo -e "\e[93m## Building installer          ##\e[0m"
	echo -e "\e[93m#################################\e[0m"
	python build_tools/installer.py
fi

echo -e "\e[93m#################################\e[0m"
echo -e "\e[93m## Done !                      ##\e[0m"
echo -e "\e[93m#################################\e[0m"

exit 0
