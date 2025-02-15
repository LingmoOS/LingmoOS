#  KCONFIG_ADD_KCFG_FILES (SRCS_VAR [GENERATE_MOC] [USE_RELATIVE_PATH] file1.kcfgc ... fileN.kcfgc)
#  KCONFIG_ADD_KCFG_FILES (<target> [GENERATE_MOC] [USE_RELATIVE_PATH] file1.kcfgc ... fileN.kcfgc) (since 5.67)
#    Use this to add KDE config compiler files to your application/library.
#    Use optional GENERATE_MOC to generate moc if you use signals in your kcfg files.
#    Use optional USE_RELATIVE_PATH to generate the classes in the build following the given
#    relative path to the file.
#    <target> must not be an alias.
#
# SPDX-FileCopyrightText: 2006-2009 Alexander Neundorf <neundorf@kde.org>
# SPDX-FileCopyrightText: 2006, 2007, Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

include(CMakeParseArguments)

function (KCONFIG_ADD_KCFG_FILES _target_or_source_var)
   set(options GENERATE_MOC USE_RELATIVE_PATH)
   cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})

   if (TARGET ${_target_or_source_var})
       get_target_property(aliased_target ${_target_or_source_var} ALIASED_TARGET)
       if(aliased_target)
           message(FATAL_ERROR "Target argument passed to kconfig_add_kcfg_files must not be an alias: ${_target_or_source_var}")
       endif()
   endif()

   set(sources)
   foreach (_current_FILE ${ARG_UNPARSED_ARGUMENTS})
       get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
       get_filename_component(_abs_PATH ${_tmp_FILE} PATH)

       if (ARG_USE_RELATIVE_PATH) # Process relative path only if the option was set
           # Get relative path
           get_filename_component(_rel_PATH ${_current_FILE} PATH)

           if (IS_ABSOLUTE ${_rel_PATH})
               # We got an absolute path
               set(_rel_PATH "")
           endif ()
       endif ()

       get_filename_component(_basename ${_tmp_FILE} NAME_WE)
       # If we had a relative path and we're asked to use it, then change the basename accordingly
       if(_rel_PATH)
           set(_basename ${_rel_PATH}/${_basename})
       endif()

       # if the file name in the kcfgc is changed, we need to rerun cmake
       set_property(DIRECTORY APPEND
           PROPERTY CMAKE_CONFIGURE_DEPENDS ${_tmp_FILE}
       )

       file(READ ${_tmp_FILE} _contents)
       string(REGEX MATCH "File=([^\n]+\\.kcfg)\n" "" "${_contents}")
       set(_kcfg_FILENAME "${CMAKE_MATCH_1}")
       if (NOT _kcfg_FILENAME)
            string(REGEX MATCH "File=([^\n]+\\.kcfg).*\n" "" "${_contents}")
            if(CMAKE_MATCH_1)
                message(WARNING "${_tmp_FILE}: Broken \"File\" field, make sure it's pointing at the right file")
                set(_kcfg_FILENAME "${CMAKE_MATCH_1}")
            else()
                message(WARNING "Couldn't read the \"File\" field in ${_tmp_FILE}")
                set(_kcfg_FILENAME "${_basename}.kcfg")
            endif()
       endif()

       string(REGEX MATCH "HeaderExtension=([^\n]+)\n" "" "${_contents}")
       if(CMAKE_MATCH_1)
            set(_kcfg_header_EXT "${CMAKE_MATCH_1}")
       else()
            set(_kcfg_header_EXT "h")
       endif()

       string(REGEX MATCH "SourceExtension=([^\n]+)\n" "" "${_contents}")
       if(CMAKE_MATCH_1)
            set(_kcfg_src_EXT "${CMAKE_MATCH_1}")
       else()
            set(_kcfg_src_EXT "cpp")
       endif()

       set(_src_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.${_kcfg_src_EXT})
       set(_header_FILE ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.${_kcfg_header_EXT})
       set(_moc_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc)
       set(_kcfg_FILE   ${_abs_PATH}/${_kcfg_FILENAME})
       # Maybe the .kcfg is a generated file?
       if(NOT EXISTS "${_kcfg_FILE}")
           set(_kcfg_FILE   ${CMAKE_CURRENT_BINARY_DIR}/${_kcfg_FILENAME})
       endif()

       if(NOT EXISTS "${_kcfg_FILE}")
           message(FATAL_ERROR "${_kcfg_FILENAME} not found; tried in ${_abs_PATH} and ${CMAKE_CURRENT_BINARY_DIR}")
       endif()

       # make sure the directory exist in the build directory
       if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}")
           file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH})
       endif()

       # the command for creating the source file from the kcfg file
       add_custom_command(OUTPUT ${_header_FILE} ${_src_FILE}
                          COMMAND KF5::kconfig_compiler
                          ARGS ${_kcfg_FILE} ${_tmp_FILE} -d ${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}
                          MAIN_DEPENDENCY ${_tmp_FILE}
                          DEPENDS ${_kcfg_FILE} KF5::kconfig_compiler)

       set_source_files_properties(${_header_FILE} ${_src_FILE} PROPERTIES
           SKIP_AUTOMOC ON
           SKIP_AUTOUIC ON
       )

       if(ARG_GENERATE_MOC)
          list(APPEND sources ${_moc_FILE})
          qt_generate_moc(${_header_FILE} ${_moc_FILE})
          set_property(SOURCE ${_src_FILE} APPEND PROPERTY OBJECT_DEPENDS ${_moc_FILE} )
       endif()

       list(APPEND sources ${_src_FILE} ${_header_FILE})
   endforeach (_current_FILE)

   if (TARGET ${_target_or_source_var})
      target_sources(${_target_or_source_var} PRIVATE ${sources})
   else()
      set(${_target_or_source_var} ${${_target_or_source_var}} ${sources} PARENT_SCOPE)
   endif()

endfunction(KCONFIG_ADD_KCFG_FILES)
