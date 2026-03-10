FROM mercenaries/mercseng:2.0

ARG TOOLCHAIN_BUILD_DIR=/building_tmp

COPY gcc-11.2.sh ${TOOLCHAIN_BUILD_DIR}/scripts/
RUN ${TOOLCHAIN_BUILD_DIR}/scripts/gcc-11.2.sh

RUN yum -y install perl-Time-Piece

RUN yum -y install mesa-libGLU-devel
