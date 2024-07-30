# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2021 Harald Sitter <sitter@kde.org>

if(NOT BUILD_TESTING)
    return()
endif()

remove_definitions(-DQT_NO_CAST_FROM_ASCII)

ecm_add_tests(cleanuptest.cpp LINK_LIBRARIES Qt::Core Qt::Test TARGET_NAMES_VAR cleanuptest_TARGET)
add_dependencies(${cleanuptest_TARGET} drkonqi-coredump-cleanup)
