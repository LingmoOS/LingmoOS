# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2005 Laurent Montel

# - Try to find the sensors directory library
# Once done this will define
#
#  SENSORS_FOUND - system has SENSORS
#  SENSORS_INCLUDE_DIR - the SENSORS include directory
#  SENSORS_LIBRARIES - The libraries needed to use SENSORS

FIND_PATH(SENSORS_INCLUDE_DIR sensors/sensors.h)

FIND_LIBRARY(SENSORS_LIBRARIES NAMES sensors)

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sensors DEFAULT_MSG SENSORS_INCLUDE_DIR SENSORS_LIBRARIES )

MARK_AS_ADVANCED(SENSORS_INCLUDE_DIR SENSORS_LIBRARIES)

