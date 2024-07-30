cmake_minimum_required(VERSION 3.16)

project(kscreenlocker)
set(CMAKE_C_STANDARD 99)
set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

set(QT_MIN_VERSION "6.6.0")
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake ${ECM_MODULE_PATH} )

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMSetupVersion)
include(ECMGenerateHeaders)
include(CheckIncludeFiles)
include(ECMMarkNonGuiExecutable)
include(CMakePackageConfigHelpers)
include(FeatureSummary)
include(GenerateExportHeader)
include(KDEClangFormat)
include(KDEGitCommitHooks)
include(ECMDeprecationSettings)

include(CheckIncludeFile)
include(CheckSymbolExists)
check_include_file("sys/prctl.h" HAVE_SYS_PRCTL_H)
check_symbol_exists(PR_SET_DUMPABLE "sys/prctl.h" HAVE_PR_SET_DUMPABLE)
check_include_file("sys/procctl.h" HAVE_SYS_PROCCTL_H)
check_symbol_exists(PROC_TRACE_CTL "sys/procctl.h" HAVE_PROC_TRACE_CTL)
if (HAVE_PR_SET_DUMPABLE OR HAVE_PROC_TRACE_CTL)
  set(CAN_DISABLE_PTRACE TRUE)
endif ()
add_feature_info("prctl/procctl tracing control"
                 CAN_DISABLE_PTRACE
                 "Required for disallowing ptrace on greeter and kcheckpass process")

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS DBus Widgets Quick Test)
find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Crash
    KCMUtils
    KIO
    GlobalAccel
    I18n
    IdleTime
    Notifications
    Solid
    WindowSystem
    XmlGui
    Svg
)

find_package(PlasmaQuick ${PROJECT_VERSION} REQUIRED)

find_package(X11)
set_package_properties(X11 PROPERTIES DESCRIPTION "X11 libraries"
                        URL "https://www.x.org"
                        TYPE REQUIRED
                        PURPOSE "Required for building the X11 based workspace")

find_package(XCB MODULE REQUIRED COMPONENTS XCB KEYSYMS XTEST)
set_package_properties(XCB PROPERTIES TYPE REQUIRED)
add_feature_info("XInput" X11_Xinput_FOUND "Required for grabbing XInput2 devices in the screen locker")

find_package(WaylandScanner)
find_package(Wayland 1.3 COMPONENTS Client Server)
find_package(LayerShellQt REQUIRED)
find_package(KF6Screen REQUIRED)

set_package_properties(Wayland PROPERTIES
                       TYPE REQUIRED
                       PURPOSE "Required for building screenlocker")

find_package(loginctl)
set_package_properties(loginctl PROPERTIES
    URL "https://www.freedesktop.org/software/systemd/man/loginctl.html"
    DESCRIPTION "Send control commands to the login manager"
    TYPE RUNTIME
)
set(HAVE_LOGINCTL ${loginctl_FOUND})

if (NOT HAVE_LOGINCTL)
    find_package(ConsoleKit)
    set_package_properties(ConsoleKit PROPERTIES
        URL "https://github.com/ConsoleKit2/ConsoleKit2"
        DESCRIPTION "Framework for tracking user login sessions"
        TYPE RECOMMENDED
    )
    set(HAVE_CONSOLEKIT ${ConsoleKit_FOUND})
endif ()

if (HAVE_LOGINCTL OR HAVE_CONSOLEKIT)
    set(HAVE_UNLOCK_CAPABILITY TRUE)
endif ()

add_feature_info("Unlock broken screenlocker"
                 HAVE_UNLOCK_CAPABILITY
                 "Needed for emergency unlock in case that the greeter is broken. In case your distribution does not provide loginctl or consolekit please contact plasma-devel@kde.org to discuss alternatives.")

option(PAM_REQUIRED "Require building with PAM" ON)

include(ConfigureChecks.cmake)

configure_file(config-unix.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-unix.h )

if (HAVE_CONSOLEKIT)
    configure_file(ck-unlock-session.cmake ${CMAKE_CURRENT_BINARY_DIR}/ck-unlock-session)
endif ()

# adjusting CMAKE_C_FLAGS to get wayland protocols to compile
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu90")

ecm_setup_version(${PROJECT_VERSION} VARIABLE_PREFIX KSCREENLOCKER
    VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kscreenlocker_version.h"
    PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/KScreenLockerConfigVersion.cmake"
    SOVERSION 5
)

configure_file(config-kscreenlocker.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kscreenlocker.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(KSLD_INCLUDEDIR "${KDE_INSTALL_INCLUDEDIR}/KScreenLocker")

include(ECMQtDeclareLoggingCategory)

ecm_set_disabled_deprecation_versions(QT 5.15.2
    KF 5.101
)

add_subdirectory(greeter)
add_subdirectory(kcm)
add_subdirectory(settings)

add_definitions(-DTRANSLATION_DOMAIN=\"kscreenlocker\")

set(screensaver_dbusXML dbus/org.freedesktop.ScreenSaver.xml)
set(kscreensaver_dbusXML dbus/org.kde.screensaver.xml)
set(powerdevilpolicyagent_xml dbus/kf6_org.kde.Solid.PowerManagement.PolicyAgent.xml)

set(ksld_SRCS
   abstractlocker.cpp
   ksldapp.cpp
   interface.cpp
   globalaccel.cpp
   x11locker.cpp
   waylandlocker.cpp
   logind.cpp
   waylandserver.cpp
   powermanagement_inhibition.cpp
   abstractlocker.h
   ksldapp.h
   interface.h
   globalaccel.h
   x11locker.h
   waylandlocker.h
   logind.h
   waylandserver.h
   powermanagement_inhibition.h
)

ecm_qt_declare_logging_category(ksld_SRCS
    HEADER kscreenlocker_logging.h
    IDENTIFIER KSCREENLOCKER
    CATEGORY_NAME kscreenlocker
    DEFAULT_SEVERITY Critical
    DESCRIPTION "KScreenLocker"
    EXPORT KSCREENLOCKER
)

qt_add_dbus_adaptor(ksld_SRCS ${screensaver_dbusXML} interface.h ScreenLocker::Interface)
qt_add_dbus_adaptor(ksld_SRCS ${kscreensaver_dbusXML} interface.h ScreenLocker::Interface kscreensaveradaptor KScreenSaverAdaptor)
qt_add_dbus_interface(ksld_SRCS ${powerdevilpolicyagent_xml} powerdevilpolicyagent)

ecm_add_wayland_server_protocol(ksld_SRCS
    PROTOCOL protocols/ksld.xml
    BASENAME ksld
)

add_library(KScreenLocker SHARED ${ksld_SRCS})

target_link_libraries(KScreenLocker
PUBLIC
   Qt::Core
PRIVATE
   settings
   Qt::DBus
   KF6::I18n
   KF6::IdleTime
   KF6::GlobalAccel
   KF6::Notifications
   KF6::CoreAddons
   KF6::ConfigGui
   KF6::Package
   KF6::WindowSystem
   X11::X11
   XCB::XCB
   XCB::KEYSYMS
   Wayland::Server
)

set(kscreenlocker_greet_bin_abs ${CMAKE_INSTALL_FULL_LIBEXECDIR}/kscreenlocker_greet)
# The file path of kscreenlocker_greet must be relative to the KScreenLocker library.
file(RELATIVE_PATH kscreenlocker_greet_bin_rel ${CMAKE_INSTALL_FULL_LIBDIR} ${kscreenlocker_greet_bin_abs})
target_compile_definitions(KScreenLocker PRIVATE
    KSCREENLOCKER_GREET_BIN_ABS="${kscreenlocker_greet_bin_abs}"
    KSCREENLOCKER_GREET_BIN_REL="${kscreenlocker_greet_bin_rel}"
)

if (X11_Xinput_FOUND)
    target_link_libraries(KScreenLocker PRIVATE X11::Xi)
endif()

target_include_directories(KScreenLocker INTERFACE "$<INSTALL_INTERFACE:${KSLD_INCLUDEDIR}>")

# Needed to compile on Arm target.
set_target_properties(KScreenLocker PROPERTIES COMPILE_FLAGS "-fPIC")

add_library(PW::KScreenLocker ALIAS KScreenLocker)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h *.c)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})
kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

generate_export_header(KScreenLocker
    BASE_NAME KScreenLocker
    EXPORT_MACRO_NAME KSCREENLOCKER_EXPORT
    EXPORT_FILE_NAME KScreenLocker/kscreenlocker_export.h
)

configure_package_config_file(ScreenSaverDBusInterfaceConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/ScreenSaverDBusInterfaceConfig.cmake
    PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
    INSTALL_DESTINATION "${KDE_INSTALL_CMAKEPACKAGEDIR}/ScreenSaverDBusInterface")

set_target_properties(KScreenLocker PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION ${PROJECT_VERSION_MAJOR})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ScreenSaverDBusInterfaceConfig.cmake
    DESTINATION "${KDE_INSTALL_CMAKEPACKAGEDIR}/ScreenSaverDBusInterface")

configure_package_config_file("${CMAKE_CURRENT_SOURCE_DIR}/KScreenLockerConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/KScreenLockerConfig.cmake"
    PATH_VARS KDE_INSTALL_DBUSINTERFACEDIR
    INSTALL_DESTINATION "${KDE_INSTALL_CMAKEPACKAGEDIR}/KScreenLocker")

if (HAVE_CONSOLEKIT)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/ck-unlock-session DESTINATION ${KDE_INSTALL_BINDIR})
endif ()

install(TARGETS KScreenLocker EXPORT KScreenLockerTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} LIBRARY)
install(EXPORT KScreenLockerTargets DESTINATION "${KDE_INSTALL_CMAKEPACKAGEDIR}/KScreenLocker" FILE KScreenLockerTargets.cmake NAMESPACE PW::)

ecm_generate_headers(KScreenLocker_CamelCase_HEADERS
    HEADER_NAMES
        KsldApp
    REQUIRED_HEADERS KScreenLocker_HEADERS)

install(
    FILES
        ${CMAKE_CURRENT_BINARY_DIR}/KScreenLocker/kscreenlocker_export.h
        ${KScreenLocker_CamelCase_HEADERS}
        ${KScreenLocker_HEADERS}
    DESTINATION
        ${KSLD_INCLUDEDIR}/KScreenLocker
    COMPONENT Devel)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/KScreenLockerConfig.cmake"
			  "${CMAKE_CURRENT_BINARY_DIR}/KScreenLockerConfigVersion.cmake"
		DESTINATION "${KDE_INSTALL_CMAKEPACKAGEDIR}/KScreenLocker"
		COMPONENT Devel)

ecm_qt_install_logging_categories(
    EXPORT KSCREENLOCKER
    FILE kscreenlocker.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

install(FILES kscreenlocker.notifyrc  DESTINATION ${KDE_INSTALL_KNOTIFYRCDIR} RENAME ksmserver.notifyrc)
install(FILES ${screensaver_dbusXML}
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR}
        RENAME kf6_org.freedesktop.ScreenSaver.xml)

install(FILES ${kscreensaver_dbusXML}
        DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR})

if(BUILD_TESTING)
   add_subdirectory(autotests)
   add_subdirectory(tests)
endif()

ki18n_install(po)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
