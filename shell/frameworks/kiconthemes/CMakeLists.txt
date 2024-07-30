cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts
project(KIconThemes VERSION ${KF_VERSION})

# ECM setup
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
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(ECMAddQch)
include(CMakeDependentOption)
include(ECMQmlModule)

set(EXCLUDE_DEPRECATED_BEFORE_AND_AT 0 CACHE STRING "Control the range of deprecated API excluded from the build [default=0].")

option(KICONTHEMES_USE_QTQUICK "Build QML plugin using Qt6Qml" ON)
option(BUILD_QCH "Build API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)" OFF)
add_feature_info(QCH ${BUILD_QCH} "API documentation in QCH format (for e.g. Qt Assistant, Qt Creator & KDevelop)")

cmake_dependent_option(BUILD_DESIGNERPLUGIN "Build plugin for Qt Designer" ON "NOT CMAKE_CROSSCOMPILING" OFF)
add_feature_info(DESIGNERPLUGIN ${BUILD_DESIGNERPLUGIN} "Build plugin for Qt Designer")

set(kiconthemes_version_header "${CMAKE_CURRENT_BINARY_DIR}/src/kiconthemes_version.h")
ecm_setup_version(PROJECT
                  VARIABLE_PREFIX KICONTHEMES
                  VERSION_HEADER "${kiconthemes_version_header}"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6IconThemesConfigVersion.cmake"
                  SOVERSION 6)

# Dependencies
set(REQUIRED_QT_VERSION 6.5.0)
find_package(Qt6Widgets ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
find_package(Qt6Svg ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)

if (KICONTHEMES_USE_QTQUICK)
  find_package(Qt6 ${REQUIRED_QT_VERSION} NO_MODULE REQUIRED Qml Quick)
endif()

# shall we use DBus?
# enabled per default on Linux & BSD systems
set(USE_DBUS_DEFAULT OFF)
if(UNIX AND NOT APPLE AND NOT ANDROID AND NOT HAIKU)
    set(USE_DBUS_DEFAULT ON)
endif()
option(USE_DBUS "Build components using DBus" ${USE_DBUS_DEFAULT})
if(USE_DBUS)
    find_package(Qt6DBus ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE)
    set(HAVE_QTDBUS ${Qt6DBus_FOUND})
endif()

find_package(KF6Archive ${KF_DEP_VERSION} REQUIRED)
find_package(KF6I18n ${KF_DEP_VERSION} REQUIRED)
find_package(KF6ConfigWidgets ${KF_DEP_VERSION} REQUIRED)
find_package(KF6WidgetsAddons ${KF_DEP_VERSION} REQUIRED)
find_package(KF6ColorScheme ${KF_DEP_VERSION} REQUIRED)

# ensure we have our icons around on all platforms that require it
if (NOT ANDROID)
  find_package(KF6BreezeIcons ${KF_DEP_VERSION} REQUIRED)
endif()

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)

ecm_set_disabled_deprecation_versions(
    QT 6.5.0
    KF 5.102
)

add_definitions(-DTRANSLATION_DOMAIN=\"kiconthemes6\")
ki18n_install(po)
add_subdirectory(src)
if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(autotests)
endif()

# create a Config.cmake and a ConfigVersion.cmake file and install them
set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6IconThemes")

if (BUILD_QCH)
    ecm_install_qch_export(
        TARGETS KF6IconThemes_QCH
        FILE KF6IconThemesQchTargets.cmake
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel
    )
    set(PACKAGE_INCLUDE_QCHTARGETS "include(\"\${CMAKE_CURRENT_LIST_DIR}/KF6IconThemesQchTargets.cmake\")")
endif()

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/KF6IconThemesConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6IconThemesConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/KF6IconThemesConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/KF6IconThemesConfigVersion.cmake"
  DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
  COMPONENT Devel
)

install(EXPORT KF6IconThemesTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE KF6IconThemesTargets.cmake NAMESPACE KF6:: )

install(FILES
   ${kiconthemes_version_header}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KIconThemes COMPONENT Devel
)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
