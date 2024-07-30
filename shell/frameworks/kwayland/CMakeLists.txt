cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "6.1.80")
project(kwayland VERSION ${PROJECT_VERSION})

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMGeneratePkgConfigFile)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMPoQmTools)
include(ECMAddQch)
include(CMakeFindFrameworks)
include(CMakePackageConfigHelpers)
include(CheckIncludeFile)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control how much of deprecated API is built [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kwayland_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/client/kwayland_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KWAYLAND
                        VERSION_HEADER "${kwayland_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KWaylandConfigVersion.cmake"
                        SOVERSION 6)

# Dependencies
find_package(Qt6Gui ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Private)
find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Concurrent)
find_package(Qt6WaylandClient ${QT_MIN_VERSION} CONFIG REQUIRED NO_MODULE COMPONENTS Private)

find_package(Wayland 1.15 COMPONENTS Client)
set_package_properties(Wayland PROPERTIES
                       TYPE REQUIRED
                      )

find_package(WaylandScanner)

find_package(WaylandProtocols 1.15)
set_package_properties(WaylandProtocols PROPERTIES TYPE REQUIRED)

find_package(EGL)
set_package_properties(EGL PROPERTIES TYPE REQUIRED)

find_package(PlasmaWaylandProtocols 1.13.0 CONFIG)
set_package_properties(PlasmaWaylandProtocols PROPERTIES TYPE REQUIRED)

# adjusting CMAKE_C_FLAGS to get wayland protocols to compile
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")

ecm_set_disabled_deprecation_versions(
    QT 6.4.0
)

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

# Subdirectories
ecm_install_po_files_as_qm(po)

add_subdirectory(src)

if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KWayland")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KWayland_QCH
        FILE KWaylandQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KWaylandQchTargets.cmake\")")
endif()

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KWaylandConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KWaylandConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KWaylandConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/KWaylandConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

install(EXPORT KWaylandTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KWaylandTargets.cmake NAMESPACE Plasma::)


install(FILES ${kwayland_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR}/KWayland COMPONENT Devel)

if (NOT WIN32)
    ecm_generate_pkgconfig_file(BASE_NAME KWaylandClient
      INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR}/KWayland
      DEPS Qt6Gui
    INSTALL)
endif()

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
