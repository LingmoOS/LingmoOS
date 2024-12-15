# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

file(GLOB_RECURSE POWER_HEADERS "./*.h")
file(GLOB_RECURSE POWER_SOURCES "./*.h" "./*.cpp")
target_sources(${LIB_NAME} PRIVATE ${POWER_SOURCES})
target_include_directories(${LIB_NAME} PRIVATE "./dbus")
