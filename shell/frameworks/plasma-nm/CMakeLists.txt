cmake_minimum_required(VERSION 3.16)

project(plasma-networkmanagement)

set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

set(QT_MIN_VERSION "6.7.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

################# set KDE specific information #################

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH})

option(BUILD_OPENCONNECT "Build OpenConnect VPN plugin" ON)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(ECMFindQmlModule)
include(ECMQmlModule)
include(KDEGitCommitHooks)
include(ECMQtDeclareLoggingCategory)

include(FeatureSummary)
include(GenerateExportHeader)

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    DBus
    Gui
    Network
    Quick
    QuickWidgets
    Widgets
)

find_package(PkgConfig REQUIRED)

if(BUILD_OPENCONNECT)
    pkg_check_modules(OPENCONNECT IMPORTED_TARGET openconnect>=3.99)
    if(NOT TARGET PkgConfig::OPENCONNECT)
        set(BUILD_OPENCONNECT OFF)
    endif()
endif()

if(BUILD_OPENCONNECT)
    find_package(Qt6WebEngineWidgets ${QT_MIN_VERSION} CONFIG REQUIRED)
endif()

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED
    ColorScheme
    Completion
    CoreAddons
    DBusAddons
    KIO
    I18n
    NetworkManagerQt
    Notifications
    Service
    Solid
    Wallet
    WidgetsAddons
    WindowSystem
    KCMUtils
    Svg
    ModemManagerQt
)

find_package(Plasma ${PROJECT_VERSION} REQUIRED)

ecm_find_qmlmodule(org.kde.prison 1.0)

find_package(KF6Kirigami ${KF6_MIN_VERSION} CONFIG)
set_package_properties(KF6Kirigami PROPERTIES
    DESCRIPTION "A QtQuick based components set"
    PURPOSE "Required at runtime by the KCMs"
    TYPE RUNTIME
)

find_package(Qca-qt6 2.1.0)
set_package_properties(Qca-qt6 PROPERTIES DESCRIPTION "Support for encryption"
                       URL "https://download.kde.org/stable/qca-qt5/"
                       TYPE REQUIRED)

find_package(KF6Prison ${KF6_MIN_VERSION})
set_package_properties(KF6Prison PROPERTIES DESCRIPTION "Prison library"
                       URL "https://commits.kde.org/prison"
                       TYPE RUNTIME
                       PURPOSE "Needed to create mobile barcodes for WiFi networks"
                      )

pkg_check_modules(NETWORKMANAGER IMPORTED_TARGET libnm>1.4.0 REQUIRED)
pkg_check_modules(MOBILEBROADBANDPROVIDERINFO mobile-broadband-provider-info)
pkg_get_variable(BROADBANDPROVIDER_DATABASE mobile-broadband-provider-info database)

find_package(QCoro6 REQUIRED COMPONENTS Core DBus)

qcoro_enable_coroutines()

add_definitions(
    -DQT_DISABLE_DEPRECATED_BEFORE=0x050f00
    -DQT_DEPRECATED_WARNINGS_SINCE=0x060000
    -DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x055800
    -DKF_DEPRECATED_WARNINGS_SINCE=0x060000
)


add_definitions(-DQT_NO_URL_CAST_FROM_STRING)

remove_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_FROM_BYTEARRAY)

add_subdirectory(applet)
add_subdirectory(kded)
add_subdirectory(kcm)
add_subdirectory(libs)
add_subdirectory(vpn)

# Enable unit testing
if (BUILD_TESTING)
    add_subdirectory(tests)
endif()

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ecm_qt_install_logging_categories(
        EXPORT PLASMANM
        FILE plasma-nm.categories
        DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
        )

ki18n_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
