# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.13)

# defines to determind whether to show invoke log at stub function.
add_definitions(-DDEBUG_STUB_INVOKE)

add_subdirectory(dtkburn)
add_subdirectory(dtkmount)
add_subdirectory(dtkio)
add_subdirectory(dtksearch)
add_subdirectory(dtkcompressor)
