# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

file(GLOB_RECURSE LOGIN_HEADERS "./*.h")
file(GLOB_RECURSE LOGIN_SOURCES "./*.h" "./*.cpp")
target_sources(${LIB_NAME} PRIVATE ${LOGIN_SOURCES})
target_include_directories(${LIB_NAME} PRIVATE "./dbus")
