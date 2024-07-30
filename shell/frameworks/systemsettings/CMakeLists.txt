cmake_minimum_required(VERSION 3.16)

set(PROJECT_VERSION "6.1.80")

project(systemsettings VERSION ${PROJECT_VERSION})
set(PROJECT_DEP_VERSION "6.0.90")
set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDEClangFormat)
include(GenerateExportHeader)
include(ECMFindQmlModule)
include(ECMQmlModule)
include(ECMQtDeclareLoggingCategory)
include(ECMDeprecationSettings)
include(KDEGitCommitHooks)

ecm_set_disabled_deprecation_versions(
    QT 5.15.4
    KF 5.103.0
)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING)
add_definitions(-DQT_USE_QSTRINGBUILDER)
add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_definitions(-DQT_NO_CAST_TO_ASCII)


find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Widgets Quick Qml QuickWidgets
)

find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Auth
    Crash
    ItemViews
    ItemModels
    KCMUtils
    I18n
    KIO
    Service
    IconThemes
    WidgetsAddons
    WindowSystem
    XmlGui
    DBusAddons
    Config
    GuiAddons # UrlHanlder handles help:/ urls
    LingmoUI
    JobWidgets
    Runner
    OPTIONAL_COMPONENTS
    DocTools
    QUIET
)

find_package(LingmoActivities ${PROJECT_DEP_VERSION} REQUIRED)

add_library(systemsettings_debug STATIC)
target_link_libraries(systemsettings_debug Qt::Core)
ecm_qt_declare_logging_category(systemsettings_debug
    HEADER systemsettings_app_debug.h
    IDENTIFIER SYSTEMSETTINGS_APP_LOG
    CATEGORY_NAME org.kde.systemsettings.app
    DESCRIPTION "systemsettings"
    EXPORT SYSTEMSETTINGS
)

add_subdirectory(app)
add_subdirectory(categories)
if(KF6DocTools_FOUND)
    add_subdirectory(doc)
    kdoctools_install(po)
endif()
add_subdirectory(runner)

ecm_qt_install_logging_categories(EXPORT SYSTEMSETTINGS FILE systemsettings.categories DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR})

ki18n_install(po)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
