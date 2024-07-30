cmake_minimum_required(VERSION 3.16)

project(libksysguard)

set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

# check with non-Plasma consumers (e.g. KDevelop) before bumping these versions to make sure the KDE CI does not break
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)

include(ECMAddTests)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMQmlModule)

include(CMakePackageConfigHelpers)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(FeatureSummary)
include(GenerateExportHeader)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED CONFIG COMPONENTS DBus Network Widgets Qml Quick)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS CoreAddons Config I18n Auth Service Package NewStuff Solid)

find_package(ZLIB REQUIRED)
set_package_properties(ZLIB PROPERTIES DESCRIPTION "Support for gzip compressed files and data streams"
                       URL "https://zlib.net/"
                       TYPE REQUIRED
                      )

check_library_exists(c clock_gettime "time.h" HAVE_CLOCK_GETTIME_C)

ecm_setup_version(${PROJECT_VERSION}
                  VARIABLE_PREFIX KSYSGUARD
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/ksysguard_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KSysGuardConfigVersion.cmake"
                  SOVERSION 10
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    find_package(NL)
    set_package_properties(NL PROPERTIES
        TYPE REQUIRED
        PURPOSE "Used for gathering socket info via the sock_diag netlink subsystem."
        URL "https://github.com/thom311/libnl/"
    )
    find_package(libpcap)
    set_package_properties(
        libpcap PROPERTIES
        TYPE RECOMMENDED
        PURPOSE "libpcap is used for per-application network usage."
    )

    if(libpcap_FOUND )
        set(BUILD_NETWORK_PLUGIN TRUE)

        find_package(Libcap)
        set_package_properties(Libcap PROPERTIES
            TYPE OPTIONAL
            PURPOSE "Needed for setting capabilities of the per-application network plugin."
        )
    endif()

    find_package(Sensors)
    set_package_properties(Sensors PROPERTIES
        TYPE REQUIRED
        PURPOSE "Used for reading hardware sensors")
endif()

option(ENABLE_KAUTH_HELPER "Build and install ksysguardprocesslist_helper KAuth helper" ON)
add_feature_info(KAUTH_HELPER ${ENABLE_KAUTH_HELPER} "Build and install ksysguardprocesslist_helper KAuth helper")

ecm_set_disabled_deprecation_versions(
    QT 5.15.0
)

add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055B00)
add_definitions(-DKF_DEPRECATED_WARNINGS_SINCE=0x060000)
add_definitions(-DQT_USE_QSTRINGBUILDER)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_subdirectory( formatter )
add_subdirectory( processcore )
add_subdirectory( sensors )
add_subdirectory( faces )
add_subdirectory( systemstats )

if(BUILD_TESTING)
    add_subdirectory( autotests )
endif()

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

set(CMAKECONFIG_INSTALL_DIR ${KDE_INSTALL_LIBDIR}/cmake/KSysGuard)
configure_package_config_file(KSysGuardConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/KSysGuardConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/KSysGuardConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/KSysGuardConfigVersion.cmake
    DESTINATION ${CMAKECONFIG_INSTALL_DIR})

install(EXPORT libksysguardLibraryTargets
        NAMESPACE KSysGuard::
        DESTINATION ${CMAKECONFIG_INSTALL_DIR}
        FILE KSysGuardLibraryTargets.cmake )

ecm_qt_install_logging_categories(
        EXPORT LIBKSYSGUARD
        FILE libksysguard.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
