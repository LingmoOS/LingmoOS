cmake_minimum_required(VERSION 3.25.0)

project(DDM
    VERSION 0.2.2
    LANGUAGES CXX C)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Set version
set(DDM_VERSION_MAJOR 0)
set(DDM_VERSION_MINOR 20)
set(DDM_VERSION_PATCH 0)
set(DDM_VERSION_STRING "${DDM_VERSION_MAJOR}.${DDM_VERSION_MINOR}.${DDM_VERSION_PATCH}")
add_compile_definitions("DDM_VERSION=\"${DDM_VERSION_STRING}\"")

# Standard installation paths
include(GNUInstallDirs)
# Macros
include(FeatureSummary)

# Options
option(BUILD_MAN_PAGES "Build man pages" OFF)
option(ENABLE_JOURNALD "Enable logging to journald" ON)
option(NO_SYSTEMD "Disable systemd support" OFF)
option(USE_ELOGIND "Use elogind instead of logind" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ECM
find_package(ECM 1.4.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH};${ECM_MODULE_PATH}")

# Definitions
add_definitions(-Wall -Wextra)

# Default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Handle build type
if(CMAKE_BUILD_TYPE MATCHES [Dd]ebug)
    message(STATUS "Debug build")
    add_definitions(-DDEBUG)
else()
    message(STATUS "Release build")
    add_definitions(-DNDEBUG)
endif()

# Default absolute paths
if(NOT DEFINED CMAKE_INSTALL_SYSCONFDIR)
    set(CMAKE_INSTALL_SYSCONFDIR "/etc")
endif()
if(NOT DEFINED CMAKE_INSTALL_LOCALSTATEDIR)
    set(CMAKE_INSTALL_LOCALSTATEDIR "/var")
endif()

# PKG-CONFIG
find_package(PkgConfig)

# PAM
find_package(PAM REQUIRED)

# XAU
pkg_check_modules(LIBXAU REQUIRED "xau")

# XCB
find_package(XCB REQUIRED)

# XKB
find_package(XKB REQUIRED)

find_package(Qt6 CONFIG REQUIRED Core DBus Gui Qml Quick QuickControls2 LinguistTools Test QuickTest)
qt_standard_project_setup(REQUIRES 6.6)

# Uninstall target
if ("${ECM_VERSION}" VERSION_LESS "1.7.0")
    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in"
                   "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
                   IMMEDIATE @ONLY)
    add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")
endif()

# systemd
if(NOT NO_SYSTEMD AND NOT USE_ELOGIND)
    pkg_check_modules(SYSTEMD "systemd")
endif()

if(SYSTEMD_FOUND)
    add_definitions(-DHAVE_SYSTEMD)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -DHAVE_SYSTEMD)

    # libsystemd-journal was merged into libsystemd in 209
    if(${SYSTEMD_VERSION} VERSION_LESS 209)
        pkg_check_modules(JOURNALD "libsystemd-journal")
    else()
        pkg_check_modules(JOURNALD "libsystemd")
    endif()

    if(ENABLE_JOURNALD)
        if(JOURNALD_FOUND)
            add_definitions(-DHAVE_JOURNALD)
            set(CMAKE_AUTOMOC_MOC_OPTIONS -DHAVE_JOURNALD)
        else()
            message(WARNING "Disable journald support for lack of libsystemd-journal")
        endif()
    endif()

    if (NOT DEFINED SYSTEMD_SYSTEM_UNIT_DIR)
        pkg_get_variable(SYSTEMD_SYSTEM_UNIT_DIR systemd systemdsystemunitdir)
    endif()

    if (NOT DEFINED SYSTEMD_SYSUSERS_DIR)
        pkg_get_variable(SYSTEMD_SYSUSERS_DIR systemd sysusersdir)
    endif()

    if (NOT DEFINED SYSTEMD_TMPFILES_DIR)
        pkg_get_variable(SYSTEMD_TMPFILES_DIR systemd tmpfilesdir)
    endif()

    set(HALT_COMMAND "/usr/bin/systemctl poweroff")
    set(REBOOT_COMMAND "/usr/bin/systemctl reboot")
else()
    set(SYSTEMD_FOUND 0)
endif()
add_feature_info("systemd" SYSTEMD_FOUND "systemd support")
add_feature_info("journald" JOURNALD_FOUND "journald support")

# elogind
if(NO_SYSTEMD AND USE_ELOGIND)
    pkg_check_modules(ELOGIND "libelogind")
endif()

if(ELOGIND_FOUND)
    add_definitions(-DHAVE_ELOGIND)
    set(CMAKE_AUTOMOC_MOC_OPTIONS -DHAVE_ELOGIND)

    set(HALT_COMMAND "/usr/bin/loginctl poweroff")
    set(REBOOT_COMMAND "/usr/bin/loginctl reboot")
endif()
add_feature_info("elogind" ELOGIND_FOUND "elogind support")

# Default behaviour if neither systemd nor elogind is used
if (NOT ELOGIND_FOUND AND NOT SYSTEMD_FOUND)
    # Set the VT on which ddm will normally appear, and the
    # commands for shutdown and reboot. On FreeBSD, there are
    # normally more getty's running than on Linux.
    if("${CMAKE_SYSTEM_NAME}" STREQUAL "FreeBSD")
        set(HALT_COMMAND "/sbin/shutdown -p now")
    else()
        set(HALT_COMMAND "/sbin/shutdown -h -P now")
    endif()
    set(REBOOT_COMMAND "/sbin/shutdown -r now")
endif()

if (SYSTEMD_FOUND)
    set(RUNTIME_DIR_DEFAULT "/run/ddm")
else()
    set(RUNTIME_DIR_DEFAULT "${CMAKE_INSTALL_LOCALSTATEDIR}/run/ddm")
endif()

# Set constants
set(DATA_INSTALL_DIR            "${CMAKE_INSTALL_FULL_DATADIR}/ddm"                CACHE PATH      "System application data install directory")
set(DBUS_CONFIG_DIR             "${CMAKE_INSTALL_FULL_SYSCONFDIR}/dbus-1/system.d"       CACHE PATH      "DBus config files directory")
set(STATE_DIR                   "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/lib/ddm"      CACHE PATH      "State directory")
set(RUNTIME_DIR                 "${RUNTIME_DIR_DEFAULT}"                            CACHE PATH      "Runtime data storage directory")
set(QML_INSTALL_DIR             "${QT_IMPORTS_DIR}"                                 CACHE PATH      "QML component installation directory")

set(SESSION_COMMAND             "${DATA_INSTALL_DIR}/scripts/Xsession"              CACHE PATH      "Script to execute when starting the X11 desktop session")
set(WAYLAND_SESSION_COMMAND     "${DATA_INSTALL_DIR}/scripts/wayland-session"       CACHE PATH      "Script to execute when starting the Wayland desktop session")

set(CONFIG_FILE                 "${CMAKE_INSTALL_FULL_SYSCONFDIR}/ddm.conf"        CACHE PATH      "Path of the ddm config file")
set(CONFIG_DIR                  "${CMAKE_INSTALL_FULL_SYSCONFDIR}/ddm.conf.d"      CACHE PATH      "Path of the ddm config directory")
set(ACCOUNTSSERVICE_DATA_DIR    "/var/lib/AccountsService"                          CACHE PATH      "Path of the accountsservice data directory")
set(SYSTEM_CONFIG_DIR           "${CMAKE_INSTALL_PREFIX}/lib/ddm/ddm.conf.d"      CACHE PATH      "Path of the system ddm config directory")
set(LOG_FILE                    "${CMAKE_INSTALL_FULL_LOCALSTATEDIR}/log/ddm.log"  CACHE PATH      "Path of the ddm log file")
set(DBUS_CONFIG_FILENAME        "org.freedesktop.DisplayManager.conf"               CACHE STRING    "Name of the ddm config file")
set(COMPONENTS_TRANSLATION_DIR  "${DATA_INSTALL_DIR}/translations"                  CACHE PATH      "Components translations directory")
set(DDM_INITIAL_VT             "1"                                                 CACHE STRING    "Initial tty to use")


# Autodetect UID_MIN and UID_MAX from /etc/login.defs
if(NOT DEFINED LOGIN_DEFS_PATH)
    set(LOGIN_DEFS_PATH "/etc/login.defs" CACHE PATH "Path to login.defs")
endif()

if(NOT EXISTS "${LOGIN_DEFS_PATH}" AND NOT DEFINED UID_MIN AND NOT DEFINED UID_MAX)
    message(FATAL_ERROR "Could not find login.defs at ${LOGIN_DEFS_PATH}. Try -DLOGIN_DEFS_PATH.")
endif()

if(NOT DEFINED UID_MIN)
    file(STRINGS "${LOGIN_DEFS_PATH}" UID_MIN LIMIT_COUNT 1 REGEX "^UID_MIN")
    string(REGEX REPLACE "^UID_MIN[\t ]+([0-9]+)" "\\1" UID_MIN "${UID_MIN}")
    if(NOT UID_MIN MATCHES "[0-9]+")
        message(WARNING "Could not detect a valid UID_MIN (got '${UID_MIN}'). You should review your login.defs file.")
        set(UID_MIN 1000)
    else()
        message(STATUS "Autodetected UID_MIN=${UID_MIN} from ${LOGIN_DEFS_PATH}")
    endif()
endif()

if(NOT DEFINED UID_MAX)
    file(STRINGS "${LOGIN_DEFS_PATH}" UID_MAX LIMIT_COUNT 1 REGEX "^UID_MAX")
    string(REGEX REPLACE "^UID_MAX[\t ]+([0-9]+)" "\\1" UID_MAX "${UID_MAX}")
    if(NOT UID_MAX MATCHES "[0-9]+")
        message(WARNING "Could not detect a valid UID_MAX (got '${UID_MAX}'). You should review your login.defs file.")
        set(UID_MAX 65000)
    else()
        message(STATUS "Autodetected UID_MAX=${UID_MAX} from ${LOGIN_DEFS_PATH}")
    endif()
endif()

# Add subdirectories
add_subdirectory(data)
add_subdirectory(services)
add_subdirectory(src)

# enable_testing()
# add_subdirectory(test)

# Display feature summary
feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)

# CMAKE MODULE
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/data/DDMConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/data/DDMConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DDM
    PATH_VARS CMAKE_INSTALL_PREFIX
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/data/AuthConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/data/AuthConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}cmake/DDM
    PATH_VARS CMAKE_INSTALL_PREFIX
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/data/CommonConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/data/CommonConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DDM
    PATH_VARS CMAKE_INSTALL_PREFIX
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/data/DDMConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/data/AuthConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/data/CommonConfig.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DDM
)
