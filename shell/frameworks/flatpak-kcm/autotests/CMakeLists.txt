# SPDX-License-Identifier: BSD-2-Clause
# SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

if(NOT BUILD_TESTING)
    return()
endif()

find_package(Qt6 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Test)
include(ECMAddTests)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)

add_custom_target(generate_overrides
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/fixtures/overrides.in ${CMAKE_CURRENT_BINARY_DIR}/fixtures/overrides
)

ecm_add_test(flatpakpermissiontest.cpp
    TEST_NAME flatpakpermissiontest
    LINK_LIBRARIES Qt6::Test kcm_flatpak_static)
set_tests_properties(flatpakpermissiontest PROPERTIES ENVIRONMENT "LANGUAGE=fr")
add_dependencies(flatpakpermissiontest generate_overrides)

ecm_add_test(FlatpakSimpleEntryTest.cpp
    TEST_NAME FlatpakSimpleEntryTest
    LINK_LIBRARIES Qt6::Test kcm_flatpak_static)

ecm_add_test(flatpakfilesystemstest.cpp
    TEST_NAME flatpakfilesystemstest
    LINK_LIBRARIES Qt6::Test kcm_flatpak_static)
