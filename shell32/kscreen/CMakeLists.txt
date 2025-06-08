cmake_minimum_required(VERSION 3.16)

project(KScreen)
set(PROJECT_VERSION "5.27.5")
set(KSCREEN_VERSION ${PROJECT_VERSION})

add_definitions("-DKSCREEN_VERSION=\"${KSCREEN_VERSION}\"")

set(QT_MIN_VERSION "5.15.2")
set(KF5_MIN_VERSION "5.102.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMConfiguredInstall)
include(ECMGenerateDBusServiceFile)
include(ECMInstallIcons)
include(ECMMarkAsTest)
include(ECMQtDeclareLoggingCategory)
include(FeatureSummary)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

find_package(Qt${QT_MAJOR_VERSION} ${QT_MIN_VERSION} REQUIRED COMPONENTS Test Sensors)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
    Config
    DBusAddons
    Declarative
    GlobalAccel
    I18n
    IconThemes
    KCMUtils
    Plasma
    PlasmaQuick
    XmlGui
)

find_package(KF5Screen ${PROJECT_VERSION} REQUIRED)

find_package(X11)
set_package_properties(X11 PROPERTIES
    DESCRIPTION "X11 libraries"
    URL "https://www.x.org"
    PURPOSE "Required for building the X11 based workspace"
    TYPE REQUIRED
)


find_package(LayerShellQt)
set_package_properties(LayerShellQt PROPERTIES
    DESCRIPTION "Layer shell Qt bindings"
    URL "https://invent.kde.org/plasma/layer-shell-qt"
    PURPOSE "Required for the screen selection osd"
    TYPE REQUIRED
)

if(X11_FOUND)
  set(HAVE_X11 1)
  find_package(X11_XCB REQUIRED)
  find_package(XCB REQUIRED COMPONENTS XCB ATOM)
  if (QT_MAJOR_VERSION EQUAL "5")
    find_package(Qt5 ${QT_MIN_VERSION} REQUIRED COMPONENTS X11Extras)
  endif()
endif()

configure_file(config-X11.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-X11.h)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.101
)


add_subdirectory(kcm)
add_subdirectory(kded)
add_subdirectory(osd)
add_subdirectory(plasmoid)
add_subdirectory(tests)
add_subdirectory(console)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ecm_qt_install_logging_categories(EXPORT KSCREEN FILE kscreen.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
