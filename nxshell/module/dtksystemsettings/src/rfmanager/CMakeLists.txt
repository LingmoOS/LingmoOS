# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

file(GLOB_RECURSE RFMANAGER_HEADERS "./*.h")
file(GLOB_RECURSE RFMANAGER_SOURCES "./*.h" "./*.cpp")
target_sources(${LIB_NAME} PRIVATE ${RFMANAGER_SOURCES})
