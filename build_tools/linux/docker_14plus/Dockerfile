FROM mercenaries/mercseng:1.1
ENV CONAN_USER_HOME=/builder

RUN yum -y install mesa-libGL-devel mesa-libGLU-devel

#setup a conan installation every user can use
RUN rm -rf /root/.conan && mkdir -p /builder/.conan/profiles \
  && printf "[settings]\nos=Linux\nos_build=Linux\narch=x86_64\narch_build=x86_64\ncompiler=gcc\ncompiler.version=9\ncompiler.libcxx=libstdc++\nbuild_type=Release\n" > /builder/.conan/profiles/default \
  && chmod 777 -R /builder
