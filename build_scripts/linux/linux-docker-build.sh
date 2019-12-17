#!/usr/bin/env bash

# MANDATORY ENV VARS:
# OPENFX: the folder to the openfx libs and includes

# Note: run the docker as root with
# docker run --rm -it "linux-openexrid-builder" bash
set -e

this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Build the docker image
docker build -t "linux-openexrid-builder" "${this_directory}/docker" --rm

nukes=""
for nuke in `ls -d /usr/local/Nuke*` ; do
	nukes="$nukes -v $nuke:$nuke"
done

docker run --rm -it \
	-u $(id -u):$(id -g) \
	-v "${this_directory}/../..:/openexrid" \
	-v "$CONAN_DATA:/conan/.conan/data" \
	$nukes \
	"linux-openexrid-builder" ./build_scripts/linux/build-conan.sh
