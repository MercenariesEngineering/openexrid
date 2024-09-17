from conans import ConanFile, CMake, tools
import os, shutil

#conan remote add conan-transit https://api.bintray.com/conan/conan/conan-transit
#conan remote add hulud https://api.bintray.com/conan/hulud/libs
#conan remote add pierousseau https://api.bintray.com/conan/pierousseau/libs

class OpenEXRIdConan(ConanFile):
    name = "OpenExrId"
    version = "1.0-beta.29"
    license = "MIT"
    url = "https://github.com/MercenariesEngineering/openexrid"
    description = "OpenEXR files able to isolate any object of a CG image with a perfect antialiazing "
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False], "build_lib": [True, False], "build_plugins": [True, False] }
    default_options = "shared=False","*:shared=False","fPIC=True","build_lib=True","build_plugins=False"
    generators = "cmake"

    def requirements(self):
        self.requires("zlib/1.2.11@mercseng/v0")
        self.requires("bzip2/1.0.8@mercseng/v0")
        self.requires("OpenFx/1.4@pierousseau/stable")

        if self.settings.os == "Linux" and self.settings.compiler == "gcc":
            if self.settings.compiler.version == 4.8:
                # Nukes 11-13
                self.requires("OpenImageIO/2.1.15.0@mercseng/stable")
                self.requires("OpenEXR/2.5.1@mercseng/stable")
                self.requires("re2/2019-06-01@pierousseau/stable")
                self.requires("libpng/1.6.37@mercseng/v0")
                self.requires("boost/1.64.0@guerilla_legacy/v0")
            elif self.settings.compiler.version == 9:
                # Nuke 14+
                self.requires("OpenImageIO/2.1.15.0@mercseng/v2")
                self.requires("OpenEXR/2.5.1@mercseng/v0")
                self.requires("re2/2019-06-01@mercseng/v1")
                self.requires("libpng/1.6.37@mercseng/v0")
                self.requires("boost/1.73.0@mercseng/v6")
                self.requires("tbb/2020.02@mercseng/v3")
        elif self.settings.compiler == "Visual Studio":
            if self.settings.compiler.version == 14:
                # Nukes 11-13
                self.requires("OpenImageIO/2.1.15.0@mercseng/v2")
                self.requires("OpenEXR/2.5.1@mercseng/v0")
                self.requires("re2/2019-06-01@mercseng/v0")
                self.requires("libpng/1.6.37@mercseng/v0")
                self.requires("boost/1.73.0@mercseng/v2")
            elif self.settings.compiler.version == 16:
                # Nuke 14+
                self.requires("OpenImageIO/2.1.15.0@mercseng/v2")
                self.requires("OpenEXR/2.5.1@mercseng/v0")
                self.requires("re2/2019-06-01@mercseng/v1")
                self.requires("libpng/1.6.37@mercseng/v0")
                self.requires("boost/1.73.0@mercseng/v6")

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
            if self.settings.compiler == "gcc" and self.settings.compiler.version == 9:
                self.options["tbb"].shared=True
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
        cmake.definitions["BUILD_LIB"] = self.options.build_lib
        cmake.definitions["BUILD_PLUGINS"] = self.options.build_plugins
        cmake.definitions["CONAN_BUILD_INFO_DIR"] = os.path.join(self.build_folder, "..", "Conan")

        if (self.settings.compiler == "Visual Studio" and self.settings.compiler.version == 16) or (self.settings.compiler == "gcc" and self.settings.compiler.version == 9):
            # Visual 2019 / gcc 9, Nuke 14+
            cmake.definitions["CMAKE_CXX_STANDARD"] = 17
            cmake.definitions["CMAKE_CXX_STANDARD_REQUIRED"] = True
        else:
            # Visual 2015 / gcc 4.8, Nukes 11-13
            cmake.definitions["CMAKE_CXX_STANDARD"] = 11
            cmake.definitions["CMAKE_CXX_STANDARD_REQUIRED"] = True

        source_dir="%s/openexrid" % self.source_folder
        if os.path.isdir(source_dir+"/openexrid"):
            cmake.configure(source_dir=source_dir)
        else:
            cmake.configure()

        targets = []
        if self.settings.compiler == "Visual Studio":
            if self.settings.compiler.version == 14:
                # Visual 2015, Nukes 11-13
                if self.options.build_plugins:
                    targets.extend(["OpenEXRIdOFX", "OpenEXRIdForNuke11.1", "OpenEXRIdForNuke11.2", "OpenEXRIdForNuke11.3", "OpenEXRIdForNuke12.0", "OpenEXRIdForNuke12.1", "OpenEXRIdForNuke12.2", "OpenEXRIdForNuke13.0", "OpenEXRIdForNuke13.2"])
            elif self.settings.compiler.version == 16:
                # Visual 2019, Nuke 14+
                if self.options.build_lib:
                    targets.extend(["LibOpenEXRId"])
                if self.options.build_plugins:
                    targets.extend(["OpenEXRIdForNuke14.0", "OpenEXRIdForNuke15.1"])
        elif self.settings.compiler == "gcc":
            if self.settings.compiler.version == 4.8:
                # gcc 4.8, Nuke 11-13
                if self.options.build_plugins:
                    targets.extend(["OpenEXRIdOFX", "OpenEXRIdForNuke11.1", "OpenEXRIdForNuke11.2", "OpenEXRIdForNuke11.3", "OpenEXRIdForNuke12.0", "OpenEXRIdForNuke12.1", "OpenEXRIdForNuke12.2", "OpenEXRIdForNuke13.0", "OpenEXRIdForNuke13.2"])
            elif self.settings.compiler.version == 9:
                # gcc 9, Nuke 14+
                if self.options.build_lib:
                    targets.extend(["LibOpenEXRId"])
                if self.options.build_plugins:
                    targets.extend(["OpenEXRIdOFX", "OpenEXRIdForNuke14.0", "OpenEXRIdForNuke15.1"])
        
        for t in targets:
            cmake.build(target=t)
        
    def package(self):
        self.copy("*.h", dst="include/openexrid", src="openexrid/openexrid")
        self.copy("*.lib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = tools.collect_libs(self)
