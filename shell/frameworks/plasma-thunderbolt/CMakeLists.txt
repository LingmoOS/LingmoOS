cmake_minimum_required(VERSION 3.16)

project(kcm_bolt)

set(CMAKE_CXX_STANDARD 14)

#################################################
# Dependencies
set(QT_MIN_VERSION "6.6.0")
set(KF6_VERSION "5.240.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

find_package(ECM ${KF6_VERSION} REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${ECM_MODULE_PATH} ${CMAKE_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(ECMQtDeclareLoggingCategory)
include(GenerateExportHeader)
include(CheckIncludeFiles)
include(CheckSymbolExists)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

kde_enable_exceptions()

#################################################
# Dependencies

find_package(Qt6Core ${QT_MIN_VERSION} REQUIRED)
find_package(Qt6Quick ${QT_MIN_VERSION} REQUIRED)
find_package(Qt6DBus ${QT_MIN_VERSION} REQUIRED)
if (BUILD_TESTING)
	find_package(Qt6Test ${QT_MIN_VERSION} REQUIRED)
endif()

find_package(KF6CoreAddons ${KF6_VERSION} REQUIRED)
find_package(KF6KCMUtils ${KF6_VERSION} REQUIRED)
find_package(KF6I18n ${KF6_VERSION} REQUIRED)
find_package(KF6DBusAddons ${KF6_VERSION} REQUIRED)
find_package(KF6Notifications ${KF6_VERSION} REQUIRED)

find_package(Bolt)
set_package_properties(Bolt PROPERTIES DESCRIPTION "Thunderbolt device manager"
                       URL "https://gitlab.freedesktop.org/bolt/bolt"
                       PURPOSE "Runtime-only dependency for Thunderbolt KCM"
                       TYPE RUNTIME)

#################################################
ecm_set_disabled_deprecation_versions(QT 6.5
    KF 5.240
)


if (BUILD_TESTING)
    add_subdirectory(autotests)
endif()
add_subdirectory(src)

################################################

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
