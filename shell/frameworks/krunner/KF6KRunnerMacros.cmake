#.rst:
# KF6KRunnerMacros
# ---------------------------
#
# This module provides the ``krunner_configure_test`` function which takes the test- and runner target as a parameter.
# This will add the compile definitions for the AbstractRunnerTest header.
# In case of DBus runners the DESKTOP_FILE parameter must be set. This is required for loading the runner from the
# metadata file.
# SPDX-FileCopyrightText: 2020 Alexander Lohnau <alexander.lohnau@gmx.de>
# SPDX-License-Identifier: BSD-2-Clause

function(krunner_configure_test TEST_TARGET RUNNER_TARGET)
    include(CMakeParseArguments)
    set(options)
    set(oneValueArgs DESKTOP_FILE)
    set(multiValueArgs)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    get_target_property(target_type ${RUNNER_TARGET} TYPE)
    if(target_type STREQUAL "EXECUTABLE")
        if(NOT ARGS_DESKTOP_FILE)
            message(FATAL_ERROR "In case of a dbus runner the DESKTOP_FILE must be provided")
        endif()
        target_compile_definitions(${TEST_TARGET}
                PRIVATE
                KRUNNER_DBUS_RUNNER_TESTING=1
                KRUNNER_TEST_DBUS_EXECUTABLE="$<TARGET_FILE:${RUNNER_TARGET}>"
                KRUNNER_TEST_DESKTOP_FILE="${ARGS_DESKTOP_FILE}"
                )
    else()
        target_compile_definitions(${TEST_TARGET}
                PRIVATE
                KRUNNER_DBUS_RUNNER_TESTING=0
                KRUNNER_TEST_RUNNER_PLUGIN_DIR="$<TARGET_FILE_DIR:${RUNNER_TARGET}>"
                KRUNNER_TEST_RUNNER_PLUGIN_NAME="$<TARGET_FILE_NAME:${RUNNER_TARGET}>"
                )
    endif()
    add_dependencies(${TEST_TARGET} ${RUNNER_TARGET})
endfunction()
