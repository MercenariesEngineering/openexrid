#!/usr/bin/env bash

# Note: run the docker as root with
# docker run --rm -it "linux-openexrid-builder" bash
set -e

this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
mkdir -p conan_data

# Build the docker image
docker build -t "linux-openexrid-builder" "$this_directory/docker" --rm

docker run --rm -it \
	-u $(id -u):$(id -g) \
	-v "$this_directory:/openexrid" \
	-v "$this_directory/conan_data:/conan/.conan/data" \
	-v "$this_directory/../openfx:/usr/local/openfx" \
	-v "/usr/local/Nuke10.5v8:/usr/local/Nuke10.5" \
	-v "/usr/local/Nuke11.2v7:/usr/local/Nuke11.2" \
	-v "/usr/local/Nuke11.3v5:/usr/local/Nuke11.3" \
	-v "/usr/local/Nuke12.0v3:/usr/local/Nuke12.0" \
	"linux-openexrid-builder" ./build-conan.sh
