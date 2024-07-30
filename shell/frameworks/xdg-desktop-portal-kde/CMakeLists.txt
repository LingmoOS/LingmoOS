cmake_minimum_required(VERSION 3.16)

set(CMAKE_C_STANDARD 99)

set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

project(xdg-desktop-portal-kde VERSION ${PROJECT_VERSION})

set(PROJECT_DEP_VERSION "6.0.90")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################# set KDE specific information #################

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMSetupVersion)
include(ECMConfiguredInstall)
include(ECMQtDeclareLoggingCategory)
include(ECMAddTests)
include(ECMFindQmlModule)

include(FeatureSummary)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Concurrent
    DBus
    PrintSupport
    QuickWidgets
    Widgets
    WaylandClient
    Quick
    QuickControls2
    Qml
    Test
)
find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS WaylandClient)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED
    CoreAddons
    Config
    I18n
    GuiAddons
    GlobalAccel
    KIO
    Kirigami
    Notifications
    Service
    WidgetsAddons
    WindowSystem
    IconThemes
    StatusNotifierItem
)

find_package(KWayland ${PROJECT_DEP_VERSION} REQUIRED)

ecm_find_qmlmodule(org.kde.iconthemes 1.0)
ecm_find_qmlmodule(org.kde.plasma.workspace.dialogs 1.0)
find_package(Wayland 1.15 REQUIRED COMPONENTS Client)
find_package(PlasmaWaylandProtocols 1.7.0 REQUIRED)

find_package(WaylandProtocols 1.25)
set_package_properties(WaylandProtocols PROPERTIES
    TYPE REQUIRED
    PURPOSE "Collection of Wayland protocols that add functionality not available in the Wayland core protocol"
    URL "https://gitlab.freedesktop.org/wayland/wayland-protocols/"
)

find_package(KIOFuse)
set_package_properties(KIOFuse PROPERTIES
    URL https://commits.kde.org/system/kio-fuse
    TYPE RUNTIME
    PURPOSE "Automatic mounting of remote URLs")

pkg_check_modules(XKB IMPORTED_TARGET xkbcommon REQUIRED)
add_feature_info(XKB XKB_FOUND "Required for converting keysyms into keycodes")

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055B00)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX XDPK
    VERSION_HEADER ${CMAKE_BINARY_DIR}/version.h)

add_subdirectory(data)
add_subdirectory(src)
add_subdirectory(autotests)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
