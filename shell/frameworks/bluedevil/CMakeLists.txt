cmake_minimum_required(VERSION 3.16)

project(bluedevil)
set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

set(PROJECT_DEP_VERSION "6.0.90")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Qml
    Widgets
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    BluezQt
    CoreAddons
    DBusAddons
    DocTools
    I18n
    JobWidgets
    KCMUtils
    KIO
    Notifications
    Service
    Svg
    WidgetsAddons
    WindowSystem
)

find_package(Lingmo ${PROJECT_DEP_VERSION} REQUIRED)

find_package(SharedMimeInfo REQUIRED)

find_package(KF6LingmoUI ${KF6_MIN_VERSION} CONFIG)
set_package_properties(KF6LingmoUI PROPERTIES
    TYPE RUNTIME
)

include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMOptionalAddSubdirectory)
include(FeatureSummary)
include(KDEClangFormat)
include(ECMSetupVersion)
include(ECMQtDeclareLoggingCategory)
include(ECMQmlModule)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050f00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055900)

include(KDEGitCommitHooks)

ecm_setup_version(${PROJECT_VERSION}
    VARIABLE_PREFIX BLUEDEVIL
    VERSION_HEADER ${CMAKE_BINARY_DIR}/version.h
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(src)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)
ecm_qt_install_logging_categories(
    EXPORT BLUEDEVIL
    FILE bluedevil.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

add_subdirectory(doc)
kdoctools_install(po)

ki18n_install(po)
