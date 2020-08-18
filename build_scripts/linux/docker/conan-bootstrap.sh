#!/bin/sh
this_directory="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

set -e

# install conan          
scl enable rh-python35 "pip install conan==1.27.1"

chmod -R 777 ${CONAN_USER_HOME}
