find_program(PHP_EXECUTABLE php)
if (PHP_EXECUTABLE)
  set(Php_FOUND TRUE)

  # list extensions
  execute_process(COMMAND ${PHP_EXECUTABLE} -m OUTPUT_VARIABLE _module_list)
  string(REGEX REPLACE "\\[[A-Za-z ]+\\]" "" _module_list ${_module_list})
  string(REPLACE "\n" ";" PHP_MODULES ${_module_list})
  list(REMOVE_ITEM PHP_MODULES "")
  message(STATUS "Found PHP modules: ${PHP_MODULES}")
endif()

# validate a list of PHP files
function(php_lint)
  if (NOT PHP_EXECUTABLE OR NOT Php_FOUND)
    return()
  endif()

  foreach(_file ${ARGN})
    get_filename_component(_file_abs ${_file} ABSOLUTE)
    get_filename_component(_ext ${_file} EXT)
    if(NOT _ext STREQUAL ".php")
        continue()
    endif()
    string(REPLACE "/" "_" _target ${_file})
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_target}.phplint
      COMMAND ${PHP_EXECUTABLE} -l ${_file_abs}
      COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/${_target}.phplint
      MAIN_DEPENDENCY ${_file_abs}
    )
    add_custom_target(${_target}_phplint ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${_target}.phplint)
  endforeach()
endfunction()
