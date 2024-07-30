
include(CMakeParseArguments)

macro(ktexttemplate_adjust_plugin_name pluginname)
  set_target_properties(${pluginname}
    PROPERTIES PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/kf6/ktexttemplate"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/kf6/ktexttemplate"
    DEBUG_POSTFIX "d"
  )
  foreach(cfg ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${cfg} CFG)
    set_target_properties(${pluginname}
      PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY_${CFG} "${CMAKE_BINARY_DIR}/kf6/ktexttemplate"
      RUNTIME_OUTPUT_DIRECTORY_${CFG} "${CMAKE_BINARY_DIR}/kf6/ktexttemplate"
      )
  endforeach()
endmacro()
