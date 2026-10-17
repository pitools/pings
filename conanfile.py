from pathlib import Path

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMakeConfigDeps, CMakeToolchain, cmake_layout

class PingsConan(ConanFile):
    name = "pings"
    version = "0.7.0"
    package_type = "application"

    settings = "os", "arch", "compiler", "build_type"

    options = {
        "with_config": [True, False],
        "build_apps": [True, False],
    }

    default_options = {
        "with_config": True,
        "build_apps": True,
    }

    def requirements(self):
        self.requires("fmt/10.2.1")
        self.requires("boost/1.85.0")

        if self.options.with_config:
            self.requires("yaml-cpp/0.8.0")

        if self.options.build_apps:
            self.requires("ftxui/7.0.3")

    def validate(self):
        check_min_cppstd(self, "20")

        if self.options.build_apps and not self.options.with_config:
            raise ConanInvalidConfiguration(
                "Current applications require with_config=True. "
                "Set build_apps=False to build without configuration."
            )

    def layout(self):
        cmake_layout(self)

    def configure(self):
        self.options["boost"].header_only = False
        self.options["boost"].without_system = False

    def generate(self):
        if self.options.build_apps:
            self._fix_ftxui_windows_layout()

        deps = CMakeConfigDeps(self)
        deps.generate()

        toolchain = CMakeToolchain(self)
        toolchain.cache_variables["PINGS_BUILD_CONFIG"] = bool(
            self.options.with_config
        )
        toolchain.cache_variables["PINGS_BUILD_APPS"] = bool(
            self.options.build_apps
        )
        if not self.options.build_apps:
            toolchain.user_presets_path = False
        toolchain.generate()

    def _fix_ftxui_windows_layout(self):
        if self.settings.os != "Windows":
            return

        ftxui = self.dependencies["ftxui"]

        libdir = (
            Path(ftxui.package_folder)
            / "lib"
            / str(self.settings.build_type)
        )

        libdir = str(libdir.resolve())

        ftxui.cpp_info.libdirs = [libdir]

        for component in ftxui.cpp_info.components.values():
            component.libdirs = [libdir]