#!/usr/bin/env python3

import argparse, os, sys, subprocess, shutil
import multiprocessing, re, pathlib, json
from contextlib import contextmanager
from datetime import datetime

parser = argparse.ArgumentParser ()

parser.add_argument ("--clean-all", action='store_true', help='Clean all conan, build, bin directories')
parser.add_argument ("--clean-build", action='store_true', help='Clean build and bin directories')
parser.add_argument ("--show-config", action='store_true', help='Show the current configuration')
parser.add_argument ("--conan-install", "-c", action='store_true', help='Install the conan dependencies (requires conan-file, conan-profile)')
parser.add_argument ("--cmake-generate", "-g", action='store_true', help='Invoke cmake and generate the build files (sln, vcxproj, makefiles ...)')
parser.add_argument ("--build", "-b", action='store_true', help='Invoke cmake --build and build')
parser.add_argument ("--installer", "-i", action='store_true', help='Generate the installer')
parser.add_argument ("--shell", type=str, help='Start a shell using the build or run conan environment')

parser.add_argument ("--conf", type=str, help='The configuration file ($MAQUINA_CONF)')
parser.add_argument ("--build-config", type=str, choices=['Debug','RelWithDebInfo','Release'], help='The build config ($MMAQUINA_BUILD_CONFIG)')
parser.add_argument ("--build-dir", type=str, help='The location of the build ($MAQUINA_BUILD_DIR)')
parser.add_argument ("--bin-dir", type=str, help='The location of the final bin directory ($MAQUINA_BIN_DIR)')
parser.add_argument ("--conan-home", type=str, help='The conan home directory (C$ONAN_HOME)')
parser.add_argument ("--conan-file", type=str, help='The conan file to use ($MAQUINA_CONAN_FILE)')
parser.add_argument ("--conan-profile", type=str, help='The conan profile to use ($MAQUINA_CONAN_PROFILE)')
parser.add_argument ("--conan-update", type=str, choices=['full', 'fast'], help='The conan update policy ($MAQUINA_CONAN_UPDATE)')
parser.add_argument ("--conan-install-dir", type=str, help='The conan dependencies install directory ($MAQUINA_CONAN_INSTALL_DIR)')
parser.add_argument ("--conan-install-config", type=str, choices=['single', 'multi'], help='The conan build configuration ($MAQUINA_CONAN_INSTALL_CONFIG)')
parser.add_argument ("--installer-dir", type=str, help='The installer directory ($MAQUINA_INSTALLER_DIR)')
parser.add_argument ("--project", type=str, help='The project name (defines BUILD_<project>) ($MAQUINA_PROJECT)')
parser.add_argument ("--ncpus", type=int, help='The number of cpus used to build')

parser.add_argument ("--nuke150", type=str, help='The path to the install of Nuke 15.0')
parser.add_argument ("--nuke151", type=str, help='The path to the install of Nuke 15.1')
parser.add_argument ("--nuke152", type=str, help='The path to the install of Nuke 15.2')
parser.add_argument ("--nuke160", type=str, help='The path to the install of Nuke 16.0')
parser.add_argument ("--nuke161", type=str, help='The path to the install of Nuke 16.1')
parser.add_argument ("--nuke170", type=str, help='The path to the install of Nuke 17.0')

_args, extra_args = parser.parse_known_args ()
args = vars(_args)
conf_file = {}

def load_arg (var_name, env_var, default_value = None):
    if var_name in args and args[var_name] != None:
        return args[var_name]
    if var_name in conf_file:
        return conf_file[var_name]
    if env_var in os.environ:
        return os.environ[env_var]
    return default_value

root_dir = sys.path[0]

build_conf_file = os.path.abspath (load_arg ("conf", "MAQUINA_CONF", os.path.join (root_dir, ".build_conf.py")))

if build_conf_file == "" or not os.path.isfile (build_conf_file):
    raise RuntimeError (f"Can't load conf file '{build_conf_file}'")

exec(open(build_conf_file).read()) 

default_conan_install_config = "single" if sys.platform == "win32" else "multi"

conf = {}
conf["project"] = load_arg ("project", "MAQUINA_PROJECT")
conf["build_dir"] = os.path.abspath (load_arg ("build_dir", "MAQUINA_BUILD_DIR", "build"))
conf["bin_dir"] = os.path.abspath (load_arg ("bin_dir", "MAQUINA_BIN_DIR", "bin"))
conf["conan_home"] = load_arg ("conan_home", "CONAN_HOME")
conf["conan_file"] = load_arg ("conan_file", "MAQUINA_CONAN_FILE")
conf["conan_profile"] = load_arg ("conan_profile", "MAQUINA_CONAN_PROFILE")
conf["conan_update"] = load_arg ("conan_update", "MAQUINA_CONAN_UPDATE", "fast")
conf["conan_install_dir"] = load_arg ("conan_install_dir", "MAQUINA_CONAN_INSTALL_DIR", os.path.join (conf["build_dir"], "conan"))
conf["conan_install_config"] = load_arg ("conan_install_config", "MAQUINA_CONAN_INSTALL_CONFIG", default_conan_install_config)
conf["build_config"] = load_arg ("build_config", "MAQUINA_BUILD_CONFIG", "Release")
conf["installer_dir"] = load_arg ("installer_dir", "MAQUINA_INSTALLER_DIR", os.path.join (conf["build_dir"], "installer"))
conf["ncpus"] = load_arg ("ncpus", "MAQUINA_NCPUS", multiprocessing.cpu_count())
conf["nuke150"] = load_arg ("nuke150", "", "")
conf["nuke151"] = load_arg ("nuke151", "", "")
conf["nuke152"] = load_arg ("nuke152", "", "")
conf["nuke160"] = load_arg ("nuke160", "", "")
conf["nuke161"] = load_arg ("nuke161", "", "")
conf["nuke170"] = load_arg ("nuke170", "", "")

default_project_conan_file = {
    "OpenEXRId": "conanfile.py",
}

default_os_conan_profile = {
    "win32": "build_tools/conan_profile_windows_vs2022",
    "linux": "build_tools/conan_profile_linux_gcc11.2",
}

if not conf["conan_file"]:
    conf["conan_file"] = default_project_conan_file[conf["project"]]
if not conf["conan_profile"]:
    conf["conan_profile"] = default_os_conan_profile[sys.platform]

def show_conf ():
    print ()
    for key, value in conf.items ():
        print (f"{key}: {value}")
    print ()

show_conf ()

build_dir = conf["build_dir"]
bin_dir = conf["bin_dir"]
conan_home = os.path.abspath (conf["conan_home"])
conan_file = os.path.abspath (conf["conan_file"])
conan_profile = os.path.abspath (conf["conan_profile"])
conan_install_dir = os.path.abspath (conf["conan_install_dir"])
project = conf["project"]

def conan_config_install_dir (build_config = "Release"):
    if conf["conan_install_config"] == "multi":
        return f"{conan_install_dir}/{build_config}"
    else:
        return f"{conan_install_dir}"

def conan_config_toolchain_file (build_config = "Release"):
    return os.path.join (conan_config_install_dir (build_config), "conan_toolchain.cmake")

def conan_config_env_script (build_config, env_type):
    if sys.platform != "win32":
        return os.path.join (conan_config_install_dir (build_config), "conan"+env_type+"env-"+build_config.lower ()+"-x86_64.sh")
    else:
        return os.path.join (conan_config_install_dir (build_config), "conan"+env_type+"env-"+build_config.lower ()+"-x86_64.bat")

def conan_capture_env (build_config, env_type):
    env_file = conan_config_env_script (build_config, env_type)
    if sys.platform != "win32":
        env_text = subprocess.run (f". {env_file} ; env", check=True, shell=True, stdout=subprocess.PIPE, text=True).stdout
    else:
        env_text = subprocess.run (f"CALL {env_file} && set", check=True, shell=True, stdout=subprocess.PIPE, text=True).stdout
    return dict ((line.split("=", 1) for line in env_text.splitlines()))

def config_build_dir (build_config = "Release"):
    return build_dir if sys.platform == "win32" else os.path.join (build_dir, build_config)

def config_bin_dir (build_config = "Release"):
    return os.path.join (bin_dir, build_config)

def env_exec_cmd (build_config, exec_type, cmd, cwd=None):
    env = conan_capture_env (build_config, exec_type)
    shell = shell=type(cmd) is str
    return subprocess.run (cmd, env=env, shell=type(cmd) is str, cwd=cwd, check=True)

def make_unix_path (path):
    return path.replace ("\\", "/")

@contextmanager
def chdir (path):
    cwd = os.getcwd ()
    os.chdir (path)
    try:
        yield cwd
    finally:
        os.chdir (cwd)

################################################################################
## Show config

if args["show_config"]:
    sys.exit ()

################################################################################
# Clean all

def clean (clean_conan = False, clean_build = False):
    def rmtree (path):
        print (f"Cleaning {path} ...")
        shutil.rmtree (path, ignore_errors=True)
    if clean_conan:
        rmtree (conan_install_dir)
    if clean_conan or clean_build:
        rmtree (config_build_dir ("Release"))
        rmtree (config_build_dir ("RelWithDebInfo"))
        rmtree (bin_dir)

if args["clean_all"]:
    clean (clean_conan = True, clean_build = True)

if args["clean_build"]:
    clean (clean_conan = False, clean_build = True)

################################################################################
## Install conan dependencies

def conan_install ():
    print ("Installing Conan dependencies ...")

    # Checks
    if not os.path.isfile (conan_file):
        raise RuntimeError (f"Conan file {conan_file} not found")
    if not os.path.isfile (conan_profile):
        raise RuntimeError (f"Conan profile {conan_profile} not found")
    if not os.path.isdir (conan_home):
        raise RuntimeError (f"Conan home {conan_home} not found")

    # Work!
    conan_update = conf["conan_update"]
    os.environ["CONAN_HOME"] = conan_home

    def conan_install_config (build_config, dependencies_config):
        _install_dir = conan_config_install_dir (build_config)
        os.makedirs (_install_dir, exist_ok=True)
        conan_args = [ 'conan', 'install', conan_file ]
        if conan_update == "full":
            conan_args.append ("--update")
        conan_args.extend ([ "--build=missing" ])
        conan_args.extend ([ "-g", "CMakeDeps", "-g", "CMakeToolchain" ])
        conan_args.extend ([ "-pr:a", conan_profile ])
        conan_args.extend ([ "-of", _install_dir ])
        conan_args.extend ([ "-s", "build_type="+dependencies_config ])
        conan_args.extend ([ "-s", "&:build_type="+build_config ])
        if build_config == "Release":
            conan_args.extend ([ "--format=json" ])
        result = subprocess.run (conan_args, check=True, stdout=subprocess.PIPE, text=True)
        if build_config == "Release":
            with open (f"{conan_install_dir}/conan_packages_build.json", "w") as pkg_info_file:
                pkg_info_file.write (result.stdout)
        # generate the env files
        if sys.platform == "win32":
            gen_env = os.path.join ("build_tools", "generate_guerilla_env.bat")
            env_script = os.path.join (_install_dir, f"conanrunenv-{build_config.lower()}-x86_64.bat")
            env_file = os.path.join (_install_dir, f"conanrunenv-{build_config.lower()}-x86_64.env")
            subprocess.run(f"{gen_env} {env_script} {env_file}", shell=True)

    subprocess.run ([ "conan", "--version" ], check=True)
    print (f"conan home: {conan_home}")
    subprocess.run ([ "conan", "remote", "list" ], check=True)

    conan_install_config ("Release", "Release")
    conan_update = "fast"
    conan_install_config ("RelWithDebInfo", "Release")

if args["conan_install"]:
    conan_install ()

################################################################################
## Generate build files from CMake

def cmake_generate ():
    print ("Generating build file from CMake ...")

    # Checks
    conan_toolchain_cmake_default = conan_config_toolchain_file ()
    if not os.path.isfile (conan_toolchain_cmake_default):
        raise RuntimeError (f"Conan toolchain '{conan_toolchain_cmake_default}' not found")

    # Work!

    cmake_generators = [ "-G", "Visual Studio 17 2022", "-A", "x64" ] if sys.platform == "win32" else [ "-G", "Unix Makefiles" ]
    cmake_options = []

    def configure_cmake (build_config):
        _build_dir = config_build_dir (build_config)
        conan_toolchain_cmake_config = conan_config_toolchain_file (build_config)
        os.makedirs (_build_dir, exist_ok=True)
        with chdir (_build_dir) as cwd:
            print (f"Configuring {build_config} CMake generator in {_build_dir}")
            cmake_args = [ "cmake", cwd ]
            cmake_args.extend (cmake_generators)
            _conan_toolchain_cmake_config = make_unix_path(conan_toolchain_cmake_config)
            _bin_dir = make_unix_path(bin_dir)
            _conan_install_dir = make_unix_path(conan_install_dir)
            cmake_args.append (f"-DCMAKE_TOOLCHAIN_FILE={_conan_toolchain_cmake_config}")
            cmake_args.extend (cmake_options)
            if sys.platform == "win32":
                cmake_args.append ("-DCMAKE_POLICY_DEFAULT_CMP0091=NEW")
            if sys.platform == "linux":
                cmake_args.append (f"-DCMAKE_BUILD_TYPE={build_config}")

            nuke150 = conf["nuke150"]
            nuke151 = conf["nuke151"]
            nuke152 = conf["nuke152"]
            nuke160 = conf["nuke160"]
            nuke161 = conf["nuke161"]
            nuke170 = conf["nuke170"]

            if nuke150 != "":
                cmake_args.append (f"-DNUKE150_DIR={nuke150}")
            if nuke151 != "":
                cmake_args.append (f"-DNUKE151_DIR={nuke151}")
            if nuke152 != "":
                cmake_args.append (f"-DNUKE152_DIR={nuke152}")
            if nuke160 != "":
                cmake_args.append (f"-DNUKE160_DIR={nuke160}")
            if nuke161 != "":
                cmake_args.append (f"-DNUKE161_DIR={nuke161}")
            if nuke170 != "":
                cmake_args.append (f"-DNUKE170_DIR={nuke170}")

            env_exec_cmd (build_config, "build", cmake_args, cwd=_build_dir)

    configure_cmake ("Release")
    if sys.platform != "win32":
        configure_cmake ("RelWithDebInfo")

if args["cmake_generate"]:
    cmake_generate ()

################################################################################
# Build

def build ():
    build_config = conf["build_config"]
    build_dir = config_build_dir (build_config)
    print (f"Building {build_config} in {build_dir} ...")

    # Checks

    if not os.path.isdir (build_dir):
        raise RuntimeError (f"build dir '{build_dir}' not found")

    # Work!

    if sys.platform == "win32":
        env_exec_cmd (build_config, "build", [ "cmake", "--build", ".", "--config", build_config ], cwd=build_dir)
    else:
        env_exec_cmd (build_config, "build", [ "cmake", "--build", ".", "-j"+str (conf["ncpus"]) ], cwd=build_dir)

if args["build"]:
    build ()

################################################################################
# Installer

def installer ():
    build_config = conf["build_config"]
    installer_dir = os.path.abspath (conf["installer_dir"])
    cfg_bin_dir = os.path.abspath (config_bin_dir (build_config))
    cfg_build_dir = os.path.abspath (config_build_dir (build_config))
    print (f"Build installer in {installer_dir} ...")

    # Checks

    # Work!

    shutil.rmtree (installer_dir, ignore_errors=True)

    if project == "Guerilla3":
        install_cmd = [ "python", os.path.join (root_dir, "Guerilla3", "build_tools", "installer", "make_bundle.py"),
            "-k", build_config,
            "--package-directory", installer_dir,
            "--bin-directory", cfg_bin_dir,
            "--build-directory", cfg_build_dir,
            "--conan-install-directory", conan_install_dir,
            "--verbose",
            "--no-sign"
            ]
        env_exec_cmd (build_config, "run", install_cmd)

    elif project == "Guerilla2":

        os.makedirs (installer_dir, exist_ok=True)

        conan_home_path = pathlib.Path (conan_home)
        run_env = conan_capture_env (build_config, "run")

        # Grab packages from conan
        packages = {}
        with open (f"{conan_install_dir}/conan_packages_build.json") as input:
            graph = json.load (input)
            for key, node in graph['graph']['nodes'].items ():
                name = node['name']
                if node['package_folder'] and not name in packages:
                    cpp_info_root = node["cpp_info"]["root"]
                    package = {
                        "package_folder": node['package_folder'],
                        "package_type": node['package_type'],
                        "includedirs": cpp_info_root["includedirs"],
                        "libdirs": cpp_info_root["libdirs"],
                        "libs": cpp_info_root["libs"],
                    }
                    packages[name] = package

        # This function scans all dll/so dependencies in all binaries in path
        def scan_all_dependencies(path):
            dependencies = []
            visited = set()

            if sys.platform == "win32":
                def is_included (lib):
                    return True
                libs_path = run_env['PATH'].split (';')
            else:
                exclude = re.compile ("libGL.so.*|libGLdispatch.so.*|libGLX.so.*|libOpenGL.so.*")
                def is_included (lib):
                    return exclude.match (lib) == None
                libs_path = run_env['LD_LIBRARY_PATH'].split (':')

            def get_dependencies (binary):
                deps = []
                if sys.platform == "win32":
                    result = subprocess.run ("dumpbin /DEPENDENTS "+binary, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
                    for line in result.stdout.splitlines ():
                        m = re.match ("\\s*(.*\\.dll)\\s*", line)
                        dep = m.group (1) if m else None
                        if dep and is_included (dep):
                            deps.append (dep)
                else:
                    for line in subprocess.run ([ "readelf", "-d", binary ], stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True).stdout.splitlines ():
                        m = re.match (".*Shared library\\: \\[(.*)\\].*", line)
                        dep = m.group (1) if m else None
                        if dep and is_included (dep):
                            deps.append (dep)
                return deps

            def scan_dependencies(binary):
                for dep in get_dependencies (binary):
                    if dep not in visited:
                        visited.add (dep)
                        for path in libs_path:
                            lib_path = os.path.join (path, dep)
                            if os.path.isfile (lib_path) and conan_home_path in pathlib.Path (lib_path).parents:
                                dependencies.append (lib_path)
                                scan_dependencies (lib_path)
                                break

            def scan_all_binary_dependencies(path):
                for entry in os.listdir (path):
                    entry_path = os.path.join (path, entry)
                    if os.path.isfile (entry_path):
                        scan_dependencies (entry_path)
                    elif os.path.isdir (entry_path) and entry != "." and entry != "..":
                        scan_all_binary_dependencies (entry_path)
            
            scan_all_binary_dependencies(path)
            return dependencies

        print ("Listing thrid party dependencies")
        dependencies = scan_all_dependencies (cfg_bin_dir)

        print (f"Writing dependencies to {installer_dir}/packages_libs.txt")
        with open (f"{installer_dir}/packages_libs.txt", mode="w") as output:
            for dep in dependencies:
                output.write (dep+"\n")
            # usd needs special care and copy the additional files
            output.write (os.path.join (packages["usd"]["libdirs"][0], "usd")+"\n")
            output.write (os.path.join (packages["usd"]["libdirs"][0], "python")+"\n")

        with chdir (os.path.join (root_dir, "build_tools", "installer")):
            lua = "lua.exe" if sys.platform == "win32" else "lua"
            subprocess.run ([ lua, f"make_{sys.platform}_package.lua",
                f"--rootdir={root_dir}",
                f"--bindir={cfg_bin_dir}",
                f"--installdir={installer_dir}",
                f"--packageslibs={installer_dir}/packages_libs.txt"
                ], check=True)

if args["installer"]:
    installer ()

################################################################################
# Shell

def shell (env_type):
    build_config = conf["build_config"]
    print (f"Start shell in {build_config} {env_type} ...")
    if sys.platform == "win32":
        env_exec_cmd(build_config, env_type, "cmd")
    else:
        env_exec_cmd(build_config, env_type, "bash")

if args["shell"]:
    shell (args["shell"])
    sys.exit (0)

