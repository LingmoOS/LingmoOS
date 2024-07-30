cmake_minimum_required(VERSION 3.16)
project(pam_kwallet)
set(KF6_MIN_VERSION "6.2.0")
set(KDE_COMPILERSETTINGS_LEVEL "5.82")
set(PROJECT_VERSION "6.1.80")
set(PROJECT_VERSION_MAJOR 6)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package (ECM ${KF6_MIN_VERSION} REQUIRED NO_MODULE)
set (CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" )

include(CheckFunctionExists)
include(KDEInstallDirs)
include(KDEClangFormat)
include(ECMConfiguredInstall)
include(KDEGitCommitHooks)

find_package(PAM REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBGCRYPT REQUIRED IMPORTED_TARGET libgcrypt)

find_program(SOCAT_EXECUTABLE socat)

if (SOCAT_EXECUTABLE)
  message (STATUS "Found socat executable: ${SOCAT_EXECUTABLE}")
else ()
  message (WARNING "socat is required for pam_kwallet to work")
endif ()

include_directories (
   ${PAM_INCLUDE_DIR}
   ${CMAKE_BINARY_DIR}
   ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x050c00)
add_definitions(-DKF_DISABLE_DEPRECATED_BEFORE_AND_AT=0x054200)

set(library_name "pam_kwallet5")

set(pam_kwallet_SRCS
   pam_kwallet.c
)

if (NOT KWALLETD_BIN_PATH)
  message (STATUS "KWALLETD_BIN_PATH not set, trying KF6::Wallet")
  find_package(KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS
    Wallet
  )
endif ()

add_definitions(-DKWALLETD_BIN_PATH="${KWALLETD_BIN_PATH}")

if (APPLE)
LIST(APPEND pam_kwallet_SRCS
   pam_darwin.c
)
endif()

# Handle different PAM header styles:
#  - "Linux style" has pam_ext.h
#  - "BSD style" has pam_appl.h
#
find_file(PAM_EXT_PATH security/pam_ext.h)
find_file(PAM_APPL_PATH security/pam_appl.h)
if (PAM_EXT_PATH)
    add_definitions(-DHAVE_PAM_EXT)
endif()
if (PAM_APPL_PATH)
    add_definitions(-DHAVE_PAM_APPL)
endif()

set(CMAKE_REQUIRED_INCLUDES "string.h")
check_function_exists(explicit_bzero HAVE_EXPLICIT_BZERO)
set(CMAKE_REQUIRED_INCLUDES)

if (HAVE_EXPLICIT_BZERO)
    add_definitions(-DHAVE_EXPLICIT_BZERO)
endif()

add_library (${library_name} SHARED ${pam_kwallet_SRCS})
set_target_properties (${library_name} PROPERTIES PREFIX "")
target_link_libraries (${library_name}
   ${PAM_LIBRARIES}
   PkgConfig::LIBGCRYPT
)

# add clang-format target for all our real source files
file(GLOB_RECURSE ALL_CLANG_FORMAT_SOURCE_FILES *.cpp *.h)
kde_clang_format(${ALL_CLANG_FORMAT_SOURCE_FILES})

kde_configure_git_pre_commit_hook(CHECKS CLANG_FORMAT)

install(TARGETS ${library_name} DESTINATION ${KDE_INSTALL_LIBDIR}/security)

configure_file(pam_kwallet_init.desktop.cmake ${CMAKE_BINARY_DIR}/pam_kwallet_init.desktop)

install(PROGRAMS pam_kwallet_init DESTINATION ${KDE_INSTALL_LIBEXECDIR})

install(FILES ${CMAKE_BINARY_DIR}/pam_kwallet_init.desktop DESTINATION ${KDE_INSTALL_AUTOSTARTDIR})

ecm_install_configured_files(INPUT plasma-kwallet-pam.service.in @ONLY DESTINATION
        ${KDE_INSTALL_SYSTEMDUSERUNITDIR})
