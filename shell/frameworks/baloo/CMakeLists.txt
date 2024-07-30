# set minimum version requirements
cmake_minimum_required(VERSION 3.16)
set(REQUIRED_QT_VERSION 6.5.0)
set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts

# set up project
project(Baloo VERSION ${KF_VERSION})

# set up extra-cmake-modules before trying anything else
include(FeatureSummary)

find_package(ECM ${KF_DEP_VERSION} NO_MODULE)
set_package_properties(ECM
    PROPERTIES
        TYPE REQUIRED
        DESCRIPTION "Extra CMake Modules"
        URL "https://commits.kde.org/extra-cmake-modules"
)
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

# we found extra-cmake-modules, so use it now
set(CMAKE_MODULE_PATH
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${ECM_MODULE_PATH}
)

include(ECMSetupVersion)

# set up baloo versions
ecm_setup_version(PROJECT
    VARIABLE_PREFIX BALOO
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/baloo_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6BalooConfigVersion.cmake"
    SOVERSION 6
)

# include optional and extra cmake stuff
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDEGitCommitHooks)
include(ECMAddTests)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateExportHeader)
include(ECMGeneratePkgConfigFile)
include(ECMConfiguredInstall)
include(ECMDeprecationSettings)
include(ECMQmlModule)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

option(BUILD_EXPERIMENTAL "Build experimental features" OFF)
add_feature_info(EXP ${BUILD_EXPERIMENTAL} "Build experimental features")

option(BUILD_INDEXER_SERVICE "Build the background indexer service" ON)
add_feature_info(INDEXER_SERVICE ${BUILD_INDEXER_SERVICE}
    "Build and install the session wide indexer service. The service is started with the session and running in the background.")

# set up build dependencies
find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core DBus Widgets Qml Quick Test)

find_package(KF6 ${KF_DEP_VERSION} REQUIRED COMPONENTS CoreAddons Config DBusAddons I18n IdleTime Solid FileMetaData Crash KIO)

find_package(LMDB)
set_package_properties(LMDB
    PROPERTIES
        DESCRIPTION "Lightning Memory-Mapped Database (LMDB)"
        URL "https://symas.com/lmdb"
        TYPE REQUIRED
)
if(${LMDB_FOUND})
    include_directories(${LMDB_INCLUDE_DIRS})
endif()

# compiler flags and build system
ecm_set_disabled_deprecation_versions(
    QT 6.4
    KF 5.97
)

find_package(Inotify)
set_package_properties(Inotify PROPERTIES
    PURPOSE "Filesystem alteration notifications using inotify")
set(BUILD_KINOTIFY ${Inotify_FOUND})

configure_file(config.h.in ${CMAKE_BINARY_DIR}/config.h)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}

    # we'll configure the rest here so that we don't have to
    # specify them in the source and test directories separately
    ${CMAKE_SOURCE_DIR}/src/lib
    ${CMAKE_BINARY_DIR}/src/lib
    ${CMAKE_BINARY_DIR}/src/codecs
    ${CMAKE_SOURCE_DIR}/src/codecs
    ${CMAKE_BINARY_DIR}/src/engine
    ${CMAKE_SOURCE_DIR}/src/engine
    ${CMAKE_BINARY_DIR}/src/file
    ${CMAKE_SOURCE_DIR}/src/file
    ${CMAKE_BINARY_DIR}/src/dbus
    ${CMAKE_SOURCE_DIR}/src/dbus
)

# targets
add_subdirectory(src)

if (BUILD_TESTING AND BUILD_KINOTIFY)
   add_subdirectory(tests)
   add_subdirectory(autotests)
endif()

# i18n
if (IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/po")
   ki18n_install(po)
endif()

# config files
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Baloo")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Baloo_QCH
        FILE KF6BalooQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6BalooQchTargets.cmake\")")
endif()

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF6BalooConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6BalooConfig.cmake"
    PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

# install targets
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF6BalooConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6BalooConfigVersion.cmake"
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    COMPONENT devel
)

install(EXPORT KF6BalooTargets
    NAMESPACE KF6::
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    FILE KF6BalooTargets.cmake)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/baloo_version.h"
    DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Baloo
    COMPONENT Devel
)

# and we're done
include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
