from conans import ConanFile, CMake
import os

#conan remote add conan-transit https://api.bintray.com/conan/conan/conan-transit
#conan remote add hulud https://api.bintray.com/conan/hulud/libs
#conan remote add pierousseau https://api.bintray.com/conan/pierousseau/libs

class OpenEXRIdConan(ConanFile):
	settings = "os", "compiler", "build_type", "arch"
	default_options = "*:shared=False"

	def requirements(self):
		# From our recipes :
		self.requires("zlib/1.2.11@pierousseau/stable")
		self.requires("IlmBase/2.2.0@pierousseau/stable")
		self.requires("OpenEXR/2.2.0@pierousseau/stable")
		self.requires("OpenImageIO/1.6.18@pierousseau/stable")
		self.requires("re2/2019-06-01@pierousseau/stable")

	def configure(self):
		if self.settings.os == "Linux":
			# fPIC option exists only on linux
			self.options["boost"].fPIC=True
			self.options["IlmBase"].fPIC=True
			self.options["OpenEXR"].fPIC=True
			self.options["OpenImageIO"].fPIC=True
			self.options["re2"].fPIC=True
			self.options["zlib"].fPIC=True
