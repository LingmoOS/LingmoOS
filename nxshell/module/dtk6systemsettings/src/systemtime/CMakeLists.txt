# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

file(GLOB_RECURSE SYSTEMTIME_HEADERS "./*.h")
file(GLOB_RECURSE SYSTEMTIME_SOURCES "./*.h" "./*.cpp")
target_sources(${LIB_NAME} PRIVATE ${SYSTEMTIME_SOURCES})
target_include_directories(${LIB_NAME} PRIVATE "./dbus")
