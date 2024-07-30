cmake_minimum_required(VERSION 3.16)

project(plasma-nano)

set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

# minimal requirements
set(PROJECT_DEP_VERSION "6.0.90")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMOptionalAddSubdirectory)
include(ECMInstallIcons)
include(ECMSetupVersion)
include(ECMMarkNonGuiExecutable)
include(ECMGenerateHeaders)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(GenerateExportHeader)
include(FeatureSummary)
include(ECMDeprecationSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED Core Gui Qml Quick Svg)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS WindowSystem I18n Service ItemModels)
find_package(Plasma ${PROJECT_DEP_VERSION} REQUIRED)
find_package(KWayland ${PROJECT_DEP_VERSION} REQUIRED)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

include(CheckIncludeFiles)

plasma_install_package(shell org.kde.plasma.nano shells)
ecm_set_disabled_deprecation_versions(QT 6.5
    KF 5.240
)


add_subdirectory(components)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)
