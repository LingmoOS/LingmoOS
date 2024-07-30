# SPDX-License-Identifier: BSD-3-Clause
# SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
# SPDX-FileCopyrightText: 2022 Harald Sitter <sitter@kde.org>

set(SELENIUM_ATSPI_MODULES_DIR ${CMAKE_CURRENT_LIST_DIR})

function(find_pythonmodule MODULE_NAME)
    set(GENMODULE "${MODULE_NAME}-PythonModule")

    configure_file("${SELENIUM_ATSPI_MODULES_DIR}/FindPythonModule.cmake.in" "Find${GENMODULE}.cmake" @ONLY)

    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_BINARY_DIR}" ${CMAKE_MODULE_PATH})
    find_package(${GENMODULE} ${ARGN})

    if(COMMAND set_package_properties)
        set_package_properties(${GENMODULE} PROPERTIES
            DESCRIPTION "Python module '${MODULE_NAME}' is a runtime dependency."
            TYPE RUNTIME)
    endif()
endfunction()
