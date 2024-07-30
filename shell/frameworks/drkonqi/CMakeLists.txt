cmake_minimum_required(VERSION 3.16)

project(drkonqi)
set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(ECMAddTests)
include(ECMMarkAsTest)
include(CheckFunctionExists)
include(FeatureSummary)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)
include(ECMFindQmlModule)
include(ECMSourceVersionControl)
include(CMakeDependentOption)
include(FindPythonModule)

option(WITH_GDB12 "The gdb version available is at least GDB 12 (this enables dynamic debug symbol resolution even when no DEBUG_PACKAGE_INSTALLER_NAME is available)" FALSE)
option(WITH_PYTHON_VENDORING "Python dependency vendoring (cmake will install python dependencies into drkonqi's python tree)" ON)

kde_enable_exceptions()

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Widgets Test DBus Concurrent Quick)

find_package(
    KF6
    ${KF6_MIN_VERSION}
    REQUIRED
    COMPONENTS
        I18n
        CoreAddons
        Config
        Service
        JobWidgets
        KIO
        Crash
        WidgetsAddons
        Wallet
        Notifications
        IdleTime
        WindowSystem
        SyntaxHighlighting
        StatusNotifierItem
)

ecm_find_qmlmodule(org.kde.lingmoui 2.20)
ecm_find_qmlmodule(org.kde.kitemmodels 1.0)
ecm_find_qmlmodule(org.kde.syntaxhighlighting 1.0)

find_package(Python3 3.8 COMPONENTS Interpreter)

find_pythonmodule(psutil)
find_pythonmodule(pygdbmi)
find_pythonmodule(sentry_sdk)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.240
)

if(CMAKE_SYSTEM_NAME MATCHES Linux)
    find_package(Systemd 249)
    set_package_properties(Systemd PROPERTIES TYPE REQUIRED PURPOSE "Coredumpd integration.")
endif()

if(Systemd_FOUND)
    find_package(PolkitQt6-1) # This is only used when extracting crashes from coredumpd
    set_package_properties(PolkitQt6-1 PROPERTIES TYPE REQUIRED PURPOSE "Reading kwin_wayland crashes")

    find_package(coredumpctl)
    set_package_properties(
        coredumpctl
        PROPERTIES
        TYPE RUNTIME
        DESCRIPTION "coredumpd CLI. part of systemd"
        PURPOSE "Required for coredumpd support to work at runtime!"
    )
endif()

if(MINGW)
    find_package(ZLIB REQUIRED)
    find_library(INTL_LIBRARY NAMES intl)
    find_library(IBERTY_LIBRARY NAMES iberty)
    find_library(BFD_LIBRARY NAMES bfd)
    if(IBERTY_LIBRARY)
        set(iberty_FOUND 1)
    else()
        set(msg "iberty")
    endif()
    if(BFD_LIBRARY)
        set(bfd_FOUND 1)
    else()
        set(msg "${msg} bfd")
    endif()
    if(INTL_LIBRARY)
        set(intl_FOUND 1)
    else()
        set(msg "${msg} intl")
    endif()
    if(msg)
        message(FATAL_ERROR "could not find ${msg}")
    endif()

    add_library(intl SHARED IMPORTED)
    set_target_properties(intl PROPERTIES IMPORTED_IMPLIB ${INTL_LIBRARY})
    add_library(iberty STATIC IMPORTED)
    set_target_properties(iberty PROPERTIES IMPORTED_LOCATION ${IBERTY_LIBRARY})
    add_library(bfd STATIC IMPORTED)
    set_target_properties(
        bfd
        PROPERTIES
            IMPORTED_LOCATION ${BFD_LIBRARY}
            # bfd header requires this to be defined
            INTERFACE_COMPILE_DEFINITIONS "PACKAGE;PACKAGE_VERSION"
    )
    find_path(BFD_INCLUDE_DIR bfd.h)
    include_directories(${BFD_INCLUDE_DIR})
endif()

include_directories("${CMAKE_CURRENT_BINARY_DIR}")

add_subdirectory(src)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

ecm_qt_install_logging_categories(
    EXPORT DRKONQI
    FILE drkonqi.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
