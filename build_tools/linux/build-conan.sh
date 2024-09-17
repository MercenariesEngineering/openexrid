#!/bin/bash

set -e

THIS_DIRECTORY="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
BUILD_ROOT=${THIS_DIRECTORY}/../..
cd ${BUILD_ROOT}

conan remote add mercenaries http://c3dcoodq1.mercs-eng.com:8081/artifactory/api/conan/conan_packages --insert 0 --force
conan remote add guerilla_legacy http://c3dcoodq1.mercs-eng.com:8081/artifactory/api/conan/guerilla_legacy --insert 1 --force
./build.sh --installer
