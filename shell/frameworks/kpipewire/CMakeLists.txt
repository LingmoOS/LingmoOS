cmake_minimum_required(VERSION 3.16)
project(KPipewire VERSION "6.1.80")

set(KF6_MIN_VERSION "6.2.0")
find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMQtDeclareLoggingCategory)

find_package(Qt6 REQUIRED COMPONENTS DBus Quick)
find_package(KF6 REQUIRED COMPONENTS I18n CoreAddons)

find_package(EGL REQUIRED)

pkg_check_modules(PipeWire IMPORTED_TARGET libpipewire-0.3 REQUIRED)

# ffmpeg, for recording
pkg_check_modules(AVCodec IMPORTED_TARGET libavcodec REQUIRED)
pkg_check_modules(AVUtil IMPORTED_TARGET libavutil REQUIRED)
pkg_check_modules(AVFormat IMPORTED_TARGET libavformat REQUIRED)
pkg_check_modules(AVFilter IMPORTED_TARGET libavfilter REQUIRED)
pkg_check_modules(GBM IMPORTED_TARGET gbm REQUIRED)
pkg_check_modules(SWScale IMPORTED_TARGET libswscale REQUIRED)
find_package(epoxy 1.3 REQUIRED)
find_package(Libdrm 2.4.62 REQUIRED)
pkg_check_modules(LIBVA IMPORTED_TARGET libva REQUIRED)
pkg_check_modules(LIBVA-drm IMPORTED_TARGET libva-drm REQUIRED)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

# Prevent EGL headers from including platform headers, in particular Xlib.h.
add_definitions(-DMESA_EGL_NO_X11_HEADERS)
add_definitions(-DEGL_NO_X11)
add_definitions(-DEGL_NO_PLATFORM_SPECIFIC_TYPES)

if (BUILD_TESTING)
   find_package(PlasmaWaylandProtocols REQUIRED)
   include(ECMAddTests)
   find_package(Qt6 REQUIRED COMPONENTS Test QuickTest)
   add_subdirectory(tests)
endif()
add_subdirectory(src)

ki18n_install(po)
