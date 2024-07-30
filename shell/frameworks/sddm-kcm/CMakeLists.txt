cmake_minimum_required(VERSION 3.16)
project(sddm-kcm)
set(PROJECT_VERSION "6.1.80")

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.5.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH}  ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(FeatureSummary)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Gui Widgets Quick QuickWidgets DBus)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    I18n
    Auth
    KIO
    Archive
    NewStuff
    KCMUtils
    Service
)

ecm_set_disabled_deprecation_versions(QT 6.4.0
    KF 5.240
)

add_definitions(-DTRANSLATION_DOMAIN=\"kcm_sddm\")
add_definitions(-DQT_NO_NARROWING_CONVERSIONS_IN_CONNECT)

kauth_install_actions(org.kde.kcontrol.kcmsddm kcm_sddm.actions)

add_executable(kcmsddm_authhelper sddmauthhelper.cpp sddmauthhelper.h)
target_link_libraries(kcmsddm_authhelper KF6::AuthCore KF6::ConfigCore KF6::Archive KF6::I18n Qt6::DBus)

kauth_install_helper_files(kcmsddm_authhelper org.kde.kcontrol.kcmsddm root)
install(TARGETS kcmsddm_authhelper DESTINATION ${KAUTH_HELPER_INSTALL_DIR})

#installer tool for knewstuff
add_executable(sddmthemeinstaller
    sddmthemeinstaller.cpp
)

target_link_libraries(sddmthemeinstaller
    KF6::I18n
    KF6::AuthCore
    KF6::CoreAddons
    KF6::ConfigCore
    KF6::WidgetsAddons
    Qt6::DBus
)
install(TARGETS sddmthemeinstaller ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(src)
install(FILES sddmtheme.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ki18n_install(po)
