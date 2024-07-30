# - Try to find DNSSD
# Once done this will define
#
#  DNSSD_FOUND - system has DNSSD
#  DNSSD_INCLUDE_DIR - the DNSSD include directory
#  DNSSD_LIBRARIES - Link these to use dnssd
#  DNSSD_DEFINITIONS - Compiler switches required for using DNSSD
#
# need more test: look at into dnssd/configure.in.in

# SPDX-FileCopyrightText: 2006 Laurent Montel <montel@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

include(CMakePushCheckState)
include(CheckFunctionExists)

FIND_PATH(DNSSD_INCLUDE_DIR dns_sd.h
  /usr/include/avahi-compat-libdns_sd/
)

if (DNSSD_INCLUDE_DIR)
  if (APPLE)
    set(DNSSD_LIBRARIES "/usr/lib/libSystem.dylib")
  else (APPLE)
    FIND_LIBRARY(DNSSD_LIBRARIES NAMES dns_sd dnssd)
  endif (APPLE)

  cmake_push_check_state()
  set(CMAKE_REQUIRED_INCLUDES ${DNSSD_INCLUDE_DIR})
  set(CMAKE_REQUIRED_LIBRARIES ${DNSSD_LIBRARIES})
  CHECK_FUNCTION_EXISTS(DNSServiceRefDeallocate DNSSD_FUNCTION_FOUND)
  cmake_pop_check_state()

  if (DNSSD_INCLUDE_DIR AND DNSSD_LIBRARIES AND DNSSD_FUNCTION_FOUND)
     set(DNSSD_FOUND TRUE)
  endif (DNSSD_INCLUDE_DIR AND DNSSD_LIBRARIES AND DNSSD_FUNCTION_FOUND)
endif (DNSSD_INCLUDE_DIR)

if (DNSSD_FOUND)
  if (NOT DNSSD_FIND_QUIETLY)
    message(STATUS "Found DNSSD: ${DNSSD_LIBRARIES}")
  endif (NOT DNSSD_FIND_QUIETLY)
else (DNSSD_FOUND)
  if (DNSSD_FIND_REQUIRED)
    message(FATAL_ERROR "Could NOT find DNSSD")
  endif (DNSSD_FIND_REQUIRED)
endif (DNSSD_FOUND)

MARK_AS_ADVANCED(DNSSD_INCLUDE_DIR DNSSD_LIBRARIES)
