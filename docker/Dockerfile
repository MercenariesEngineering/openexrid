FROM centos:6.6

# Initial bootstrapping
COPY bootstrap.sh /build/
RUN /build/bootstrap.sh

ENV CONAN_USER_HOME=/conan
COPY conan_profile_linux ${CONAN_USER_HOME}/.conan/profiles/default
COPY get-pip.py /build/
COPY extra-bootstrap.sh /build/
RUN /build/extra-bootstrap.sh

ENV CMAKE_ROOT=/usr/local/cmake-3.10.2
ENV CMAKE_MODULE_PATH=${CMAKE_ROOT}/share/cmake-3.10/Modules
ENV PATH=${CMAKE_ROOT}/bin:${PATH}
ENV PATH=/usr/local/patch-2.7.6/bin:${PATH}
ENV PATH=/usr/local/binutils-2.30/bin:${PATH}

COPY extra-bootstrap.2.sh /build/
RUN /build/extra-bootstrap.2.sh

COPY entrypoint.sh /build/
ENTRYPOINT [ "/build/entrypoint.sh" ]
