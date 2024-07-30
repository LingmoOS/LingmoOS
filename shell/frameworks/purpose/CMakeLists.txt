cmake_minimum_required(VERSION 3.16)

project(Purpose)

find_package(ECM 6.4.0 REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

set(REQUIRED_QT_VERSION 6.5.0)
include(KDEInstallDirs)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(KDEGitCommitHooks)

include(FeatureSummary)

include(ECMGenerateExportHeader)
include(ECMSetupVersion)
include(ECMAddTests)
include(ECMInstallIcons)
include(ECMGenerateHeaders)
include(ECMMarkNonGuiExecutable)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMGenerateQmlTypes)
include(ECMQMLModules)
include(ECMQmlModule)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Core Qml Gui Widgets Network Test)

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
  find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED DBus)
  set(HAVE_DBUS ON)
endif()

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts

find_package(KF6 ${KF_DEP_VERSION} REQUIRED COMPONENTS CoreAddons I18n Config Kirigami Notifications KIO Service)

find_package(KAccounts6)

set_package_properties(KAccounts6 PROPERTIES
                       PURPOSE "Needed for YouTube and Nextcloud upload"
                       TYPE OPTIONAL
)
ecm_find_qmlmodule(SSO.OnlineAccounts 0.1)

ecm_find_qmlmodule(org.kde.kquickcontrolsaddons 2.0)
ecm_find_qmlmodule(org.kde.prison 1.0)

ecm_setup_version(${KF_VERSION} VARIABLE_PREFIX PURPOSE
                        VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/purpose_version.h"
                        PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6PurposeConfigVersion.cmake"
                        SOVERSION 6)

ecm_set_disabled_deprecation_versions(
    QT 6.5.0
    KF 5.240
)

add_subdirectory( src )
if (BUILD_TESTING)
    add_subdirectory( autotests )
    add_subdirectory( tests )
endif()

ki18n_install(po)

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6Purpose")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6PurposeConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PurposeConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PurposeConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6PurposeConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6PurposeTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6PurposeTargets.cmake NAMESPACE KF6:: )

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/purpose_version.h
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/Purpose COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL   FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
