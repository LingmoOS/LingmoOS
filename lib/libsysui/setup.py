from time import time, gmtime, strftime
import os
import pathlib
import platform
import subprocess
import sys
from setuptools import setup
from setuptools import Extension
from setuptools.command.build_ext import build_ext

MODULE_NAME = 'LingmoUIPy'


class CMakeExtension(Extension):
    """
    自定义了 Extension 类，忽略原来的 sources、libraries 等参数，交给 CMake 来处理这些事情
    """

    def __init__(self, name: str, sourcedir: str = ""):
        super().__init__(name, sources=[])
        self.sourcedir = os.fspath(pathlib.Path(sourcedir).resolve())


class BuildExt(build_ext):
    """
    自定义了 build_ext 类，对 CMakeExtension 的实例，调用 CMake 和 Make 命令来编译它们
    """

    def build_extension(self, ext: CMakeExtension) -> None:
        cwd = pathlib.Path().absolute()

        build_temp = f"{pathlib.Path(self.build_temp)}/{ext.name}"
        os.makedirs(build_temp, exist_ok=True)

        # Must be in this form due to bug in .resolve() only fixed in Python 3.10+
        ext_fullpath = pathlib.Path.cwd() / self.get_ext_fullpath(ext.name)
        extdir = ext_fullpath.parent.resolve() / MODULE_NAME

        debug = int(os.environ.get("DEBUG", 0)
                    ) if self.debug is None else self.debug
        config = "Debug" if debug else "Release"

        cmake_args = [
            "-S", str(cwd),
            "-B", str(build_temp),
            "-DBUILD_PYTHON=ON",
            "-DCMAKE_CXX_FLAGS_INIT:STRING=",
            "-DCMAKE_GENERATOR:STRING=Ninja",
            "-DCMAKE_BUILD_TYPE:STRING=" + config,
            "-DCMAKE_INSTALL_PREFIX:PATH=" + str(extdir),
        ]

        # 如果是Windows，强制使用CL.exe 作为C_compiler
        if platform.system() == "Windows":
            cmake_args += [
                "-DCMAKE_C_COMPILER=cl.exe",
                "-DCMAKE_CXX_COMPILER=cl.exe",
            ]

        build_args = [
            "--config", config,
            "--", "-j8"
        ]

        env = os.environ.copy()  # 获取当前环境变量副本
        subprocess.run(["cmake"] + cmake_args, check=True, env=env)
        if not self.dry_run:
            subprocess.run(["cmake", "--build", str(build_temp)
                            ] + build_args, check=True, env=env)
            subprocess.run(
                ["cmake", "--install", str(build_temp)], check=True, env=env)


def check_qt_version():
    # using subprocess to get the version of qt using CMake
    # result = subprocess.run(["cmake", "--find-package", "-DNAME=Qt", "-DCOMPILER_ID=GNU", "-DLANGUAGE=CXX", "-DMODE=VERSION"], capture_output=True, text=True)
    # using files in $(cwd)/.cmake/CheckForQtVersion.cmake
    # it will output a file to ${CMAKE_BINARY_DIR}/qt_version.txt "${QT_VERSION}"

    # Configure the CMake command
    cwd = pathlib.Path().absolute()

    build_temp = f"{pathlib.Path(cwd)}" + "/build/qt_version_checker"
    os.makedirs(build_temp, exist_ok=True)

    debug = None
    config = "Debug" if debug else "Release"

    cmake_args = [
        "-S", str(cwd) + "/.cmake/CheckForQtVersion",
        "-B", str(build_temp),
        "-DBUILD_PYTHON=ON",
        "-DCMAKE_CXX_FLAGS_INIT:STRING=",
        "-DCMAKE_GENERATOR:STRING=Ninja",
        "-DCMAKE_BUILD_TYPE:STRING=" + config,
    ]

    # 如果是Windows，强制使用CL.exe 作为C_compiler
    if platform.system() == "Windows":
        cmake_args += [
            "-DCMAKE_C_COMPILER=cl.exe",
            "-DCMAKE_CXX_COMPILER=cl.exe",
        ]

    build_args = [
        "--config", config,
        "--", "-j8"
    ]

    env = os.environ.copy()  # 获取当前环境变量副本
    subprocess.run(["cmake"] + cmake_args, check=True, env=env)

    try:
        with open(str(build_temp) + "/qt_version.txt", 'r') as file:
            qt_version = file.read().strip()
            print(f"Qt Version: {qt_version}")
            return qt_version
    except FileNotFoundError:
        print(f"Error: File {str(build_temp)}/qt_version.txt not found.")
        return None
    except Exception as e:
        print(f"Unexpected error: {e}")
        return None


lingmoui = CMakeExtension("LingmoUI")

qt_version = check_qt_version()

if qt_version is None:
    print("Warning: Qt version not found, please install Qt and try again.")
    qt_version = "6.7.3"

# Get time in YYYMMDDHHMMSS format using time module
# 获取当前时间的时间戳
current_time = time()
# 将时间戳转换为UTC时间
utc_time = gmtime(current_time)
# 格式化为 YYYYMMDDHH
formatted_time = strftime('%Y%m%d%H', utc_time)


setup(name="LingmoUIPy",
      version="3.1.3b" + formatted_time,
      description="This is LingmoUI for Python",
      long_description="LingmoUI is designed to provide a modern, consistent user interface toolkit for desktop applications. It extends the standard Qt Quick Controls with custom styling, additional components, and integrated window management features. The library enables developers to create visually appealing applications with minimal effort, handling common UI requirements like theming, window decorations, and responsive layouts.",
      ext_modules=[lingmoui],
      packages=['LingmoUIPy'],
      cmdclass={"build_ext": BuildExt},  # 使用自定义的 build_ext 类
      install_requires=[
          f"pyside6=={qt_version}",
      ],
      license = "GPL-3.0-or-later",
      classifiers=[
          "Environment :: Console",
          "Environment :: MacOS X",
          "Environment :: X11 Applications :: Qt",
          "Environment :: Win32 (MS Windows)",
          "Intended Audience :: Developers",
          "Operating System :: MacOS :: MacOS X",
          "Operating System :: POSIX",
          "Operating System :: POSIX :: Linux",
          "Operating System :: Microsoft",
          "Operating System :: Microsoft :: Windows",
          "Programming Language :: C++",
          "Programming Language :: Python",
          "Programming Language :: Python :: 3",
          "Programming Language :: Python :: 3.9",
          "Programming Language :: Python :: 3.10",
          "Programming Language :: Python :: 3.11",
          "Programming Language :: Python :: 3.12",
          "Programming Language :: Python :: 3.13",
          "Topic :: Database",
          "Topic :: Software Development",
          "Topic :: Software Development :: Code Generators",
          "Topic :: Software Development :: Libraries :: Application Frameworks",
          "Topic :: Software Development :: User Interfaces",
          "Topic :: Software Development :: Widget Sets",
      ],
      python_requires=">=3.9"
      )
