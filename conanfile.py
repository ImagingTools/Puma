#
# For using sparse checkout from the local repo set
# git config --global uploadpack.allowFilter true
#

from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout
from conan.tools.env import Environment, VirtualBuildEnv, VirtualRunEnv
from conan.tools.build import can_run
from conan.tools.files import collect_libs, update_conandata, move_folder_contents, copy
from conan.tools.scm import Git
from conan.errors import ConanInvalidConfiguration, ConanException

import os
import sys
from pathlib import Path
import json


class PumaConan(ConanFile):
    name = "puma"
    settings = "os", "compiler", "build_type", "arch"
    package_type = "application"
    options = {
        "qt_package": ["system", "conan"],
        "qt_version": "ANY",
        "BUILD_TESTING": [True, False]
    }

    default_options = {
        "qt_package": "conan",
        "qt_version": None,
        "BUILD_TESTING": False
    }

    description = "Puma is a centralized authentication and authorization framework"
    url = "https://github.com/ImagingTools/Puma"
    license = "LicenseRef-ACF-Commercial", "LGPL-2.1-or-later"
    author = "ImagingTools"
    topics = ("qt", "component-framework", "authentication", "authorization")
    generators = "CMakeDeps"
    python_requires = "conantools/0.2.4.rev.0@gmg/stable"

    @property
    def _gmgtools(self):
        return self.python_requires["conantools"].module

    def set_version(self):
        if self.version is not None:
            self.output.warning(f'Version overriden')
            return self.version
        git = Git(self)
        try:
            self.version = 'git.' + git.get_commit()[:8]
            if git.is_dirty():
                self.version += '-dirty'
            self.output.info(f'Detected version {self.version}')
        except ConanException:
            self.output.error('Unable to detect version')
            self.version = 'unknown'

    def export(self):
        git = Git(self)
        url, commit = git.get_url_and_commit()
        subfolder = self.recipe_path.relative_to(Path(git.get_repo_root())).as_posix()
        # we store the current url and commit in conandata.yml
        update_conandata(self, {"sources": {"commit": commit, "url": url, "subfolder": subfolder}})

    def validate(self):
        if self.settings.arch == "x86":
            raise ConanInvalidConfiguration("Not supported arch")

    def requirements(self):
        if self.options.qt_package == "conan":
            self.requires("qt/[>=6.8]@gmg/system")

        self.requires("imtcore/[~1]@gmg/stable")
        self.requires("acf/[~1]@gmg/stable")
        self.requires("acfsln/[~1]@gmg/stable")
        self.requires("libpq/15.5")

    def build_requirements(self):
        if self.settings.os == "Linux":
            self.build_requires("patchelf/0.13")
        self.tool_requires("nodejs/[>=16]")

    def configure(self):
        if self.options.qt_package == "system":
            self.options.qt_version = self._gmgtools.qmake_query("QT_VERSION")
            self.output.info(f"Auto-detected Qt version: {self.options.qt_version}")
        else:
            del self.options.qt_version

    def layout(self):
        cmake_layout(self, build_folder='out', src_folder='.', generator='Ninja')

    def source(self):
        git = Git(self)
        sources = self.conan_data["sources"]
        url = sources["url"]
        commit = sources["commit"]
        subfolder = sources["subfolder"]
        git.run('init')
        git.run(f'remote add origin "{url}"')
        git.run(f'fetch --depth=1 --filter=tree:0 origin "{commit}"')
        git.run(f'sparse-checkout set "{subfolder}"')
        git.checkout(commit)

        # move_folder_contents works only one level down, iterate through levels
        for sub in os.path.split(subfolder):
            self.output.info(f'moving {sub} -> {self.source_folder}')
            move_folder_contents(self, sub, self.source_folder)

    def _print_env(self, env):
        for variable, value in env.items():
            self.output.info(f'{variable}={value}')

    def _write_env_to_presets(self, env):
        presetsPath = self.generators_path / 'CMakePresets.json'
        with open(presetsPath) as f:
            data = json.load(f)
        for conf in data['configurePresets']:
            if conf['name'].endswith(str(self.settings.build_type).lower()):
                if 'environment' not in conf:
                    conf['environment'] = {}
                for var, val in env.items():
                    conf['environment'][var] = val
        with open(presetsPath, 'w') as f:
            json.dump(data, f, indent=2)

    def _get_qt_version(self):
        if self.options.qt_package == "system":
            return self.options.qt_version
        else:
            return str(self.dependencies["qt"].ref.version)

    def _update_version(self):
        script_name = "UpdateVersion.bat" if self.settings.os == "Windows" else "UpdateVersion.sh"
        script_path = os.path.join(self.source_folder, "Build", "Git", script_name)
        backup_dir = os.path.join(self.build_folder, "xtrsvn-backups")

        self.run(f'"{script_path}" "{backup_dir}"', cwd=self.source_folder)

    def _copy_dlls(self):
        binDir = Path(self.build_folder) / 'Puma' / 'Bin' / self._build_folder_suffix()
        self.output.info(f"Copying shared libraries to {binDir}")
        files = []

        for dep in self.dependencies.values():
            # ignore Acf/ImtCore bin folder contents
            # Qt is handled by windeployqt
            if dep.ref.name in ('acf', 'acfsln', 'iacf', 'imtcore', 'qt'):
                continue

            dep_info = dep.cpp_info.aggregated_components()
            for src in dep_info.bindirs:
                # Copies packages' dll files to the respective "bin" directory
                files += copy(self, '*.dll', src=src, dst=binDir)

        self.output.info(f"Copied: {[os.path.basename(f) for f in files]}")

    def generate(self):
        if self.options.qt_package == "conan":
            qtDir = str(self.dependencies["qt"].cpp_info.bindirs[0])
        else:
            qtDir = self._gmgtools.detect_qtdir(self)
        self.output.info(f"QTDIR: {qtDir}")

        tc = CMakeToolchain(self, generator='Ninja')
        tc.user_presets_path = 'Build/CMake'
        tc.variables["CMAKE_FIND_PACKAGE_PREFER_CONFIG"] = True
        tc.variables["USE_FIND_PACKAGE"] = True
        tc.variables["CMAKE_CXX_STANDARD"] = 17
        tc.variables['BUILDDIR'] = self.build_path.as_posix()
        tc.variables['PYTHONEXE'] = Path(sys.executable).as_posix()
        # fvisibility hidden
        tc.variables['CMAKE_POLICY_DEFAULT_CMP0063'] = 'NEW'
        tc.variables['CMAKE_VISIBILITY_INLINES_HIDDEN'] = True
        tc.variables['CMAKE_CXX_VISIBILITY_PRESET'] = 'hidden'
        tc.generate()

        env = Environment()
        env.define('QTDIR', qtDir)
        env = env.vars(self, scope="build")
        env.save_script("acfenv")

        buildEnv = VirtualBuildEnv(self)
        buildEnv.generate()
        self._print_env(buildEnv.environment().vars(self))

        env = buildEnv.environment().compose_env(env).vars(self)
        self._write_env_to_presets({
            var: val for var, val in env.items()
            if var.startswith('ACF') or var.startswith('IACF') or var.startswith('IMTCORE')})

        # Acf shall be both build_requires() and requires() dependency because of the Arxc compiler tool
        # However, we want to make things simple and use only requires()
        if not can_run(self):
            self.output.error("Cross compiling may not be able to run Arxc")
        if self.settings.os == "Linux":
            # On win and mac Acf tools are bundled with deployqt tool and do not require linker env variables to locate dependencies
            # On Linux we use VirtualRunEnv at build stage. This does not work with cross compiling
            runEnv = VirtualRunEnv(self)
            runEnv.generate(scope="build")

        self._update_version()
        if self.settings.os == "Windows":
            self._copy_dlls()

    def build(self):
        cmake = CMake(self)
        cmake.configure(build_script_folder='Build/CMake')
        cmake.build()

    def package_id(self):
        # Acf may break binary compatibility with any change in the version
        self.info.requires['acf'].full_version_mode()
        self.info.requires['acfsln'].full_version_mode()
        self.info.requires['imtcore'].full_version_mode()
        # qt may break binary compatibility with any change in the version
        # because of patches or repackaging
        self.info.requires['qt'].full_version_mode()

    def _build_folder_suffix(self, binary=True):
        # folder naming in acf and imt
        if self.settings.build_type == 'Release':
            build_type = 'Release'
        else:
            build_type = 'Debug'

        qt_major = self._get_qt_version().split(".")[0]

        if self.settings.os == 'Windows':
            if self.settings.compiler == 'msvc':
                compiler = 'VC' + {
                    "192": "16",
                    "193": "17", 
                    "194": "17",
                }[str(self.settings.compiler.version)]
            elif self.settings.compiler == 'Visual Studio':
                compiler = 'VC' + str(self.settings.compiler.version)
            else:
                compiler = 'Clang'
        elif self.settings.os == 'Linux':
            if self.settings.compiler == 'gcc':
                compiler = 'GCCLinux'
            else:
                compiler = 'ClangLinux'
        else:
            compiler = 'ClangOSX'

        if self.settings.arch == 'x86_64':
            arch = 'x64'
        elif self.settings.arch == 'armv8':
            arch = 'arm64'
        else:
            arch = 'x86'

        if binary:
            return f'{build_type}_Qt{qt_major}_{compiler}_{arch}'
        else:
            return f'Qt{qt_major}_{compiler}_{arch}'

    def _include_folder_suffix(self):
        return self._build_folder_suffix(binary=False)

    def package(self):
        # TODO: package() is not used at the moment and can be incomplete
        def cp(patterns, path):
            for p in patterns:
                copy(self, p, src=os.path.join(self.source_folder, path), dst=os.path.join(self.package_folder, path))

        cp(["*.*"], "Config")
        cp(["*.*"], "Partitura")
        cp(["*.*"], "Sdl")
        cp(["*.h", "*.svg", "*.css", "*.theme", "*.cpp"], "Include")
        cp(["*.h"], "Impl")

        cp(["*.h", "*.cpp", "*.qml", "*.qrc", "qmldir"], "AuxInclude")
        cp(["*.lib", "*.pdb", "*.a"], "Lib")
        copy(self, "*.pdb",
             src=os.path.join(self.source_folder, "AuxInclude"),
             dst=os.path.join(self.package_folder, "Lib", self._build_folder_suffix()),
             keep_path=False)
        cp(["*.exe", "*.dll", "*.pdb", "*.arp", "*Puma*"], "Bin")

        # copy MAC OS bundles
        cp(["*.app/*"], "Bin")

        if self.settings.os == 'Linux':
            from shlex import join
            bin_subdir = self.package_path / 'Bin' / self._build_folder_suffix()
            self.output.info(f"Processing: {bin_subdir}")
            assert os.path.isdir(bin_subdir)
            # installation is done not with cmake, remove rpath manually
            for file in bin_subdir.iterdir():
                self.output.info(f"Removing rpath from {file.name}")
                self.run(join(["patchelf", "--remove-rpath", str(file)]))

        cp(["*"], "Qml")
        cp(["*"], "3rdParty")
        cp(["*"], "Install")

    def _collect_libs(self):
        prefix = ''
        if (self.package_path / self.folders.build).is_dir():
            self.output.info("Assuming editable mode")
            prefix = self.folders.build

        libs = []
        for libdir in self.cpp_info.libdirs:
            absoluteLibdir = self.package_path / prefix / libdir
            if not absoluteLibdir.is_dir():
                raise Exception(f"libdir does not exists {absoluteLibdir}")
            libs += collect_libs(self, os.path.join(prefix, libdir))
        return libs

    def package_info(self):
        # Final executable package_info() is not required
        pass
