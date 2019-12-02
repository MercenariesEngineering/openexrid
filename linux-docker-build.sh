#!/usr/bin/env bash

# Note: run the docker as root with
# docker run --rm -it "linux-openexrid-builder" bash
set -e

this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
mkdir -p conan_data

# Build the docker image
docker build -t "linux-openexrid-builder" "$this_directory/docker" --rm

nukes=""
for nuke in `ls -d /usr/local/Nuke*` ; do
	nukes="$nukes -v $nuke:$nuke"
done

docker run --rm -it \
	-u $(id -u):$(id -g) \
	-v "$this_directory:/openexrid" \
	-v "$this_directory/conan_data:/conan/.conan/data" \
	-v "$this_directory/../openfx:/usr/local/openfx" \
	$nukes \
	"linux-openexrid-builder" ./build-conan.sh
