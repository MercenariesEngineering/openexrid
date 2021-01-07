from conans import ConanFile, CMake, tools
import os, shutil

#conan remote add conan-transit https://api.bintray.com/conan/conan/conan-transit
#conan remote add hulud https://api.bintray.com/conan/hulud/libs
#conan remote add pierousseau https://api.bintray.com/conan/pierousseau/libs

class OpenEXRIdConan(ConanFile):
    name = "OpenExrId"
    version = "1.0-beta.22"
    license = "MIT"
    url = "https://github.com/MercenariesEngineering/openexrid"
    description = "OpenEXR files able to isolate any object of a CG image with a perfect antialiazing "
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False] }
    default_options = "shared=False","*:shared=False","fPIC=True"
    generators = "cmake"

    def requirements(self):
        self.requires("zlib/1.2.11@mercseng/v0")
        self.requires("OpenFx/1.4@pierousseau/stable")
        if (self.settings.compiler == "Visual Studio" and self.settings.compiler.version == 10) or (self.settings.compiler == "gcc" and self.settings.compiler.version == 4.1):
            # Building for old Nukes
            self.requires("OpenImageIO/1.6.18@pierousseau/stable")
            self.requires("IlmBase/2.2.0@pierousseau/stable")
            self.requires("OpenEXR/2.2.0@pierousseau/stable")
            self.requires("re2/2016-02-01@pierousseau/stable")
            self.requires("libpng/1.6.37@pierousseau/stable")
            self.requires("boost/1.67.0@conan/stable")
        elif self.settings.os == "Linux":
            # Newer Nukes, Linux
            self.requires("OpenImageIO/2.1.15.0@mercseng/stable")
            self.requires("OpenEXR/2.5.1@mercseng/stable")
            self.requires("re2/2019-06-01@pierousseau/stable")
            self.requires("libpng/1.6.37@mercseng/v0")
            self.requires("boost/1.67.0@conan/stable")
        else:
            # Newer Nukes, Windows
            self.requires("OpenImageIO/2.1.15.0@mercseng/v2")
            self.requires("OpenEXR/2.5.1@mercseng/v0")
            self.requires("re2/2019-06-01@mercseng/v0")
            self.requires("libpng/1.6.37@mercseng/v0")
            self.requires("boost/1.73.0@mercseng/v2")

    def configure(self):
        if self.settings.os == "Linux":
            # fPIC option exists only on linux
            self.options["boost"].fPIC=True
            if self.settings.compiler == "gcc" and self.settings.compiler.version == 4.1:
                self.options["IlmBase"].fPIC=True
            self.options["OpenEXR"].fPIC=True
            #self.options["OpenFx"].fPIC=True
            self.options["OpenImageIO"].fPIC=True
            self.options["re2"].fPIC=True
            self.options["zlib"].fPIC=True

        if self.settings.compiler == "gcc" and self.settings.compiler.version == 4.1:
            self.options["libpng"].hardware_optimizations=False

        if (self.settings.compiler == "Visual Studio" and self.settings.compiler.version == 10) or (self.settings.os == "Linux"):
            pass
        else:
            self.options["boost"].i18n_backend = "iconv"
            self.options["boost"].zstd = True
            self.options["boost"].lzma = True
            self.options["boost"].without_python = False
            self.options["cpython"].shared=True

    def source(self):
        self.run("git clone http://github.com/MercenariesEngineering/openexrid.git --branch %s" % self.version)

    def build(self):
        cmake = CMake(self)
        #cmake.verbose = True
        cmake.definitions["USE_CONAN"] = True
        cmake.definitions["BUILD_LIB"] = True
        cmake.definitions["BUILD_PLUGINS"] = False
        cmake.configure(source_dir="%s/openexrid" % self.source_folder)
        cmake.build()
        
    def package(self):
        self.copy("*.h", dst="include/openexrid", src="openexrid/openexrid")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
