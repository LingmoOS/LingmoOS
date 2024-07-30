cmake_minimum_required(VERSION 3.16)

project(plasma-volume-control)

set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

set(PROJECT_DEP_VERSION "6.0.90")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/ ${ECM_MODULE_PATH})

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_pulseaudio\" -DQT_NO_KEYWORDS)
include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMSetupVersion)
include(ECMOptionalAddSubdirectory)
include(FindPkgConfig)
include(ECMDeprecationSettings)
include(ECMQmlModule)
include(ECMGenerateExportHeader)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX PLASMAPA
    VERSION_HEADER ${CMAKE_BINARY_DIR}/version.h)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Core
    Gui
    DBus
    Quick
    Widgets
)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Config
    CoreAddons
    Declarative
    DocTools
    GlobalAccel
    StatusNotifierItem
    I18n
    KCMUtils
    Svg
    DBusAddons
)

find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)

find_package(Canberra REQUIRED)
find_package(KF6PulseAudioQt 1.5.0 REQUIRED)

pkg_check_modules(LIBPULSE libpulse REQUIRED IMPORTED_TARGET)
pkg_check_modules(LIBPULSE_MAINLOOP libpulse-mainloop-glib REQUIRED IMPORTED_TARGET)

pkg_check_modules(GIO gio-2.0 REQUIRED IMPORTED_TARGET)

find_package(CanberraPulse)
set_package_properties(CanberraPulse PROPERTIES
    DESCRIPTION "Pulseaudio backend for libcanberra"
    PURPOSE "Required for volume feedback sounds"
    TYPE RUNTIME
)

find_package(SoundThemeFreedesktop)
set_package_properties(SoundThemeFreedesktop PROPERTIES
    DESCRIPTION "The standard freedesktop sound theme"
    PURPOSE "Required for volume feedback sounds"
    URL "https://www.freedesktop.org/wiki/Specifications/sound-theme-spec/"
    TYPE RUNTIME
)

find_package(KF6Kirigami ${KF6_MIN_VERSION} CONFIG)
set_package_properties(KF6Kirigami PROPERTIES
    DESCRIPTION "A QtQuick based components set"
    PURPOSE "Required at runtime the KCM"
    TYPE RUNTIME
)

find_package(KirigamiAddons CONFIG)
set_package_properties(KirigamiAddons PROPERTIES
    DESCRIPTION "Extra controls for Kirigami applications"
    PURPOSE "Required at runtime for the KCM"
    TYPE RUNTIME
)

find_package(KItemModels CONFIG)
set_package_properties(KItemModels PROPERTIES
    DESCRIPTION "Set of item models extending the Qt model-view framework"
    PURPOSE "Required at runtime for the KCM and applet"
    TYPE RUNTIME
)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.240.0
)

add_subdirectory(applet)
add_subdirectory(src)
add_subdirectory(doc)
add_subdirectory(appiumtests)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
