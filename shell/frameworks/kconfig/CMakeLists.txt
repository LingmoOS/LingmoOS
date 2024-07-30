cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
project(KConfig VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://commits.kde.org/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)
include(ECMPoQmTools)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)

set(REQUIRED_QT_VERSION 6.5.0)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Xml)
option(KCONFIG_USE_GUI "Build components using Qt6Gui" ON)
if(KCONFIG_USE_GUI)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Gui)
endif()

option(KCONFIG_USE_QML "Build components using Qt6Qml" ON)

if(KCONFIG_USE_QML)
    include(ECMQmlModule)
    find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Qml Quick)
endif()

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
    find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
    set(KCONFIG_USE_DBUS ON)
endif()

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kconfig_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/core/kconfig_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KCONFIG
                        VERSION_HEADER "${kconfig_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6ConfigConfigVersion.cmake"
                        SOVERSION 6)

ecm_set_disabled_deprecation_versions(
    QT 6.5
)

add_subdirectory(src)
if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()

include (ECMPoQmTools)
ecm_install_po_files_as_qm(poqm)


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Config")

if(BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6Config_QCH
        FILE KF6ConfigQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6ConfigQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KF6ConfigConfig.cmake.in"
                              "${CMAKE_CURRENT_BINARY_DIR}/KF6ConfigConfig.cmake"
                              INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
                              )

install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/KF6ConfigConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/KF6ConfigConfigVersion.cmake"
                "${CMAKE_CURRENT_SOURCE_DIR}/KF6ConfigMacros.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )


install(EXPORT KF6ConfigTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        FILE KF6ConfigTargets.cmake NAMESPACE KF6:: COMPONENT Devel)


install(EXPORT KF6ConfigCompilerTargets DESTINATION "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Config"
        FILE KF6ConfigCompilerTargets.cmake NAMESPACE KF6:: COMPONENT Devel)

install(FILES ${kconfig_version_header}
        DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KConfig COMPONENT Devel)


include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
