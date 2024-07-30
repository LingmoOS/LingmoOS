cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KCMUtils VERSION ${KF_VERSION})

include(FeatureSummary)
find_package(ECM 6.4.0  NO_MODULE)
set_package_properties(ECM PROPERTIES TYPE REQUIRED DESCRIPTION "Extra CMake Modules." URL "https://invent.kde.org/frameworks/extra-cmake-modules")
feature_summary(WHAT REQUIRED_PACKAGES_NOT_FOUND FATAL_ON_MISSING_REQUIRED_PACKAGES)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(CMakePackageConfigHelpers)
include(ECMAddQch)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMMarkNonGuiExecutable)
include(KDEGitCommitHooks)
include(ECMQmlModule)


include(CMakeDependentOption)
option(TOOLS_ONLY "Only build the tools and not the rest of KCMUtils" OFF)
if (TOOLS_ONLY)
    find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Core)
    add_subdirectory(tools)
    return()
endif()

set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Widgets Qml Quick QuickWidgets Test)

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
    find_package(Qt6DBus ${REQUIRED_QT_VERSION} CONFIG REQUIRED)
    set(HAVE_QTDBUS ${Qt6DBus_FOUND})
endif()

option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

set(kcmutils_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kcmutils_version.h")
ecm_setup_version(PROJECT VARIABLE_PREFIX KCMUTILS
                        VERSION_HEADER "${kcmutils_version_header}"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6KCMUtilsConfigVersion.cmake"
                        SOVERSION 6)

find_package(KF6KIO ${KF_DEP_VERSION} REQUIRED)
find_package(KF6ItemViews ${KF_DEP_VERSION} REQUIRED)
find_package(KF6ConfigWidgets ${KF_DEP_VERSION} REQUIRED)
find_package(KF6CoreAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6GuiAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6XmlGui ${KF_DEP_VERSION} REQUIRED)
find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)

ecm_set_disabled_deprecation_versions(
    QT 6.7.0
    KF 5.102.0
)

add_definitions(-DTRANSLATION_DOMAIN=\"kcmutils6\")
ki18n_install(po)
add_subdirectory(src)
add_subdirectory(tools)
if(BUILD_TESTING)
add_subdirectory(autotests)
endif()


# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6KCMUtils")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6KCMUtils_QCH
        FILE KF6KCMUtilsQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6KCMUtilsQchTargets.cmake\")")
endif()

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6KCMUtilsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6KCMUtilsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6KCMUtilsConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6KCMUtilsConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/KF6KCMUtilsMacros.cmake
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kcmutilsgeneratemoduledata.cpp.in
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kcmutilsgeneratemoduledata.h.in
    DESTINATION ${CMAKECONFIG_INSTALL_DIR}
)

install(EXPORT KF6KCMUtilsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6KCMUtilsTargets.cmake NAMESPACE KF6:: )

install(FILES
   ${kcmutils_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KCMUtils COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
