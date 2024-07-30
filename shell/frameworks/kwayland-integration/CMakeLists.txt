cmake_minimum_required(VERSION 3.16)
project(kwayland-integration)

set(CMAKE_C_STANDARD 99)

set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

set(QT_MIN_VERSION "5.15.2")
set(KF_MIN_VERSION "5.102.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF_MIN_VERSION} REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} )

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Widgets
    WaylandClient
)

# required frameworks
find_package(KF5 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    WindowSystem
)

find_package(QtWaylandScanner REQUIRED)
find_package(Qt5XkbCommonSupport REQUIRED) # Needed by Qt5::WaylandClientPrivate
find_package(KF5Wayland ${KF_MIN_VERSION} REQUIRED)

find_package(WaylandProtocols 1.21 REQUIRED)
find_package(Wayland 1.15 COMPONENTS Client)
find_package(PkgConfig REQUIRED)
find_package(PlasmaWaylandProtocols REQUIRED)


check_cxx_source_compiles("
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    const int size = 10;
    int fd = memfd_create(\"test\", MFD_CLOEXEC | MFD_ALLOW_SEALING);
    ftruncate(fd, size);
    fcntl(fd, F_ADD_SEALS, F_SEAL_SHRINK | F_SEAL_SEAL);
    mmap(nullptr, size, PROT_WRITE, MAP_SHARED, fd, 0);
    return 0;
}" HAVE_MEMFD)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.101
)

add_subdirectory(src)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
