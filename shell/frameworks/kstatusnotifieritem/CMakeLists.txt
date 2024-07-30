cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KStatusNotifierItem VERSION ${KF_VERSION})

# ECM setup
include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)

include(KDEGitCommitHooks)
include(GenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQtDeclareLoggingCategory)
include(ECMPoQmTools)
include(ECMAddQch)
include(ECMGenerateExportHeader)
include(ECMDeprecationSettings)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kstatusnotifieritem_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kstatusnotifieritem_version.h")
ecm_setup_version(PROJECT
  VARIABLE_PREFIX KSTATUSNOTIFIERITEM
  VERSION_HEADER "${kstatusnotifieritem_version_header}"
  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6StatusNotifierItemConfigVersion.cmake"

                        SOVERSION 6)

# Dependencies
set(REQUIRED_QT_VERSION 6.5.0)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Widgets)

if (NOT APPLE AND NOT WIN32)
    option(WITHOUT_X11 "Build without X11 support (skips finding X11)." OFF)
    if(NOT WITHOUT_X11)
        find_package(X11)
        set(HAVE_X11 ${X11_FOUND})
    endif()
endif()

find_package(KF6WindowSystem ${KF_DEP_VERSION} REQUIRED)

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
    set(HAVE_DBUS TRUE)
endif()

remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

ecm_install_po_files_as_qm(poqm)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.240
)

add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6StatusNotifierItem")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6StatusNotifierItem_QCH
        FILE KF6StatusNotifierItemQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6StatusNotifierItemQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6StatusNotifierItemConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6StatusNotifierItemConfig.cmake"
  PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6StatusNotifierItemConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6StatusNotifierItemConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6StatusNotifierItemTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6StatusNotifierItemTargets.cmake NAMESPACE KF6:: )

install(FILES
   ${kstatusnotifieritem_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KStatusNotifierItem COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
