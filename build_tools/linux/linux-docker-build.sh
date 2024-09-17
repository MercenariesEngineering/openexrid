#!/usr/bin/env bash

# MANDATORY ENV VARS:
# OPENFX: the folder to the openfx libs and includes

# Note: run the docker as root with
# docker run --rm -it "linux-openexrid-builder" bash
set -e

this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Build the docker images
docker build -t "openexrid-9-13-builder" "${this_directory}/docker_9_13" --rm

docker build -t "openexrid-14plus-builder" "${this_directory}/docker_14plus" --rm

nukes=""
for nuke in `ls -d /usr/local/Nuke*` ; do
	nukes="$nukes -v $nuke:$nuke"
done

docker run --rm -it \
	-u $(id -u):$(id -g) \
	-v "${this_directory}/../..:/openexrid" \
	-v "$CONAN_USER_HOME/.conan/data:/conan/.conan/data" \
	-e build48=1 \
	$nukes \
	"openexrid-9-13-builder" "scl enable sclo-git212 /openexrid/build_tools/linux/build-conan.sh"

docker run --rm -it \
	-u $(id -u):$(id -g) \
	-v "${this_directory}/../..:/openexrid" \
	-v "$CONAN_USER_HOME/.conan/data:/builder/.conan/data" \
	-e build93=1 \
	$nukes \
	"openexrid-14plus-builder" "/openexrid/build_tools/linux/build-conan.sh"
