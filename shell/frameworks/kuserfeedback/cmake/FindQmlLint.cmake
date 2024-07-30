# SPDX-FileCopyrightText: 2016-2017 Klarälvdalens Datakonsult AB, a KDAB Group company <info@kdab.com>
# SPDX-FileContributor: Volker Krause <volker.krause@kdab.com>
#
# SPDX-License-Identifier: BSD-3-Clause

find_program(QMLLINT_EXECUTABLE qmllint)
if (QMLLINT_EXECUTABLE AND NOT Qt5Core_VERSION VERSION_LESS 5.4)
  set(QmlLint_FOUND TRUE)
endif()

# validate a list of qml files
function(qml_lint)
  if (NOT QMLLINT_EXECUTABLE OR NOT QmlLint_FOUND)
    return()
  endif()

  foreach(_file ${ARGN})
    get_filename_component(_file_abs ${_file} ABSOLUTE)
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}.qmllint
      COMMAND ${QMLLINT_EXECUTABLE} ${_file_abs}
      COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/${_file}.qmllint
      MAIN_DEPENDENCY ${_file_abs}
    )
    add_custom_target(${_file}_qmllint ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_file}.qmllint)
  endforeach()
endfunction()
