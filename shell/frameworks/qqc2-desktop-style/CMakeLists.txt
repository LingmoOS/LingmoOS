cmake_minimum_required(VERSION 3.16)

set(KF_VERSION "6.5.0") # handled by release scripts
set(KF_DEP_VERSION "6.4.0") # handled by release scripts

project(qqc2-desktop-style VERSION ${KF_VERSION})

set(REQUIRED_QT_VERSION 6.5.0)

find_package(ECM 6.4.0 REQUIRED NO_MODULE)

# where to look first for cmake modules, before ${CMAKE_ROOT}/Modules/ is checked
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(ECMQmlModule)
include(ECMPoQmTools)
include(ECMFindQmlModule)
include(KDECMakeSettings)
include(KDEGitCommitHooks)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMDeprecationSettings)

find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Core Quick Gui Widgets QuickControls2)

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

if (BUILD_TESTING)
    include(ECMAddTests)
    find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Test QuickTest)
endif()

find_package(KF6 ${KF_DEP_VERSION} REQUIRED COMPONENTS Config KirigamiPlatform)

# IconThemes and ColorScheme are optional
find_package(KF6IconThemes ${KF_DEP_VERSION})
find_package(KF6ColorScheme ${KF_DEP_VERSION})

if (NOT APPLE AND NOT WIN32)
    find_package(X11)
endif()

set_package_properties(IconThemes
    PROPERTIES
        DESCRIPTION "KF6 IconThemes framework"
        TYPE OPTIONAL
        PURPOSE "Optional integration of Kirigami with KIconLoader icons handling for a better integration with Plasma Desktop"
)
set_package_properties(ColorScheme
    PROPERTIES
        DESCRIPTION "KF6 ColorScheme framework"
        TYPE OPTIONAL
        PURPOSE "Optional integration of Kirigami with KColorScheme for color palettes integration with Plasma Desktop"
)

find_package(PkgConfig)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6QQC2DesktopStyle")

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/KF6QQC2DesktopStyleConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6QQC2DesktopStyleConfig.cmake"
    INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR}
    PATH_VARS CMAKE_INSTALL_PREFIX
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/KF6QQC2DesktopStyleConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/KF6QQC2DesktopStyleConfigVersion.cmake"
    DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
    COMPONENT Devel
)

ecm_setup_version(PROJECT
    VARIABLE_PREFIX QQC2DESKTOPSTYLE
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KF6QQC2DesktopStyleConfigVersion.cmake"
    SOVERSION 6
)

ecm_find_qmlmodule(org.kde.sonnet 1.0)

ecm_set_disabled_deprecation_versions(
    QT 6.5
    KF 5.102
)

add_subdirectory(plugin)

if (KF6IconThemes_FOUND AND KF6ColorScheme_FOUND)
    add_subdirectory(kirigami-plasmadesktop-integration)
endif()

ecm_add_qml_module(org_kde_desktop URI "org.kde.desktop" VERSION 1.0 GENERATE_PLUGIN_SOURCE DEPENDENCIES "org.kde.qqc2desktopstyle.private" "org.kde.desktop.private")

ecm_create_qm_loader(org_kde_desktop_QM_LOADER qqc2desktopstyle_qt)
target_sources(org_kde_desktop PRIVATE ${org_kde_desktop_QM_LOADER})

file(GLOB _public_qml "org.kde.desktop/*.qml")
ecm_target_qml_sources(org_kde_desktop
    VERSION 1.0
    SOURCES ${_public_qml}
)

# Private module goes separately, because a type can not be both internal and singleton (QTBUG-111545)
ecm_add_qml_module(org_kde_desktop_private
    URI "org.kde.desktop.private"
    VERSION 1.0
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES "org.kde.qqc2desktopstyle.private"
)

set_source_files_properties(
    org.kde.desktop/private/GlobalSonnetSettings.qml
    org.kde.desktop/private/MobileTextActionsToolBar.qml
    org.kde.desktop/private/TextFieldContextMenu.qml

    PROPERTIES
        QT_QML_SINGLETON_TYPE TRUE
)

file(GLOB _private_qml "org.kde.desktop/private/*.qml")
ecm_target_qml_sources(org_kde_desktop_private
    VERSION 1.0
    SOURCES ${_private_qml}
)

ecm_finalize_qml_module(org_kde_desktop         DESTINATION ${KDE_INSTALL_QMLDIR})
ecm_finalize_qml_module(org_kde_desktop_private DESTINATION ${KDE_INSTALL_QMLDIR})

# See https://bugreports.qt.io/browse/QTBUG-123216
add_dependencies(org_kde_desktop qqc2desktopstyleplugin org_kde_desktop_private)

ecm_install_po_files_as_qm(poqm)

include(ECMFeatureSummary)
ecm_feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
