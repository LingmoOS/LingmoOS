#  KCONFIG_ADD_KCFG_FILES (SRCS_VAR [GENERATE_MOC] [USE_RELATIVE_PATH] file1.kcfgc ... fileN.kcfgc)
#  KCONFIG_ADD_KCFG_FILES (<target> [GENERATE_MOC] [USE_RELATIVE_PATH] file1.kcfgc ... fileN.kcfgc) (since 5.67)
#    Use this to add KDE config compiler files to your application/library.
#    Use optional GENERATE_MOC to generate moc if you use signals in your kcfg files.
#    Use optional USE_RELATIVE_PATH to generate the classes in the build following the given
#    relative path to the file.
#    <target> must not be an alias.
#
#  kconfig_target_kcfg_file(<target> FILE <kcfg file> CLASS_NAME <classname> ...)
#    Add a kcfg file to a target without having a kcfgc file. The kcfgc file will
#    be generated from the function arguments. Arguments correspond to the
#    similarly names fields from the kcfgc file.
#
#    TARGET should be a valid target. FILE is required and should be the name of
#    a kcfg file. CLASS_NAME is required and should be the name of the C++ class
#    that is generated. All the other arguments are optional.
#
#    Option arguments are:
#        MUTATORS
#        SINGLETON
#        CUSTOM_ADDITIONS
#        NOTIFIERS
#        DEFAULT_VALUE_GETTERS
#        ITEM_ACCESSORS
#        SET_USER_TEXTS
#        GLOBAL_ENUMS
#        USE_ENUM_TYPES
#        FORCE_STRING_FILENAME
#        GENERATE_PROPERTIES
#        PARENT_IN_CONSTRUCTOR
#        GENERATE_MOC
#    Single value arguments are:
#        FILE
#        CLASS_NAME
#        VISIBILITY
#        HEADER_EXTENSION
#        SOURCE_EXTENSION
#        NAMESPACE
#        INHERITS
#        MEMBER_VARIABLES
#        TRANSLATION_SYSTEM
#        TRANSLATION_DOMAIN
#    Multi value arguments are:
#        INCLUDE_FILES
#        SOURCE_INCLUDE_FILES
#
# SPDX-FileCopyrightText: 2006-2009 Alexander Neundorf <neundorf@kde.org>
# SPDX-FileCopyrightText: 2006, 2007, Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

include(CMakeParseArguments)

function(KCONFIG_ADD_KCFG_FILES _target_or_source_var)
    set(options GENERATE_MOC USE_RELATIVE_PATH)
    cmake_parse_arguments(ARG "${options}" "" "" ${ARGN})

    if(TARGET ${_target_or_source_var})
        get_target_property(aliased_target ${_target_or_source_var} ALIASED_TARGET)
        if(aliased_target)
            message(FATAL_ERROR "Target argument passed to kconfig_add_kcfg_files must not be an alias: ${_target_or_source_var}")
        endif()
    endif()

    set(sources)
    foreach(_current_FILE ${ARG_UNPARSED_ARGUMENTS})
        get_filename_component(_tmp_FILE ${_current_FILE} ABSOLUTE)
        get_filename_component(_abs_PATH ${_tmp_FILE} PATH)

        if(ARG_USE_RELATIVE_PATH) # Process relative path only if the option was set
            # Get relative path
            get_filename_component(_rel_PATH ${_current_FILE} PATH)

            if(IS_ABSOLUTE ${_rel_PATH})
                # We got an absolute path
                set(_rel_PATH "")
            endif()
        endif()

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
        if(NOT _kcfg_FILENAME)
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

        set(_src_FILE    ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.cpp)
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
                          COMMAND KF6::kconfig_compiler
                          ARGS ${_kcfg_FILE} ${_tmp_FILE} -d ${CMAKE_CURRENT_BINARY_DIR}/${_rel_PATH}
                          MAIN_DEPENDENCY ${_tmp_FILE}
                          DEPENDS ${_kcfg_FILE} KF6::kconfig_compiler)

        set_source_files_properties(${_header_FILE} ${_src_FILE} PROPERTIES
            SKIP_AUTOUIC ON
        )

        if(NOT ARG_GENERATE_MOC)
            set_source_files_properties(${_header_FILE} ${_src_FILE} PROPERTIES
                SKIP_AUTOMOC ON
            )
        endif()

        list(APPEND sources ${_src_FILE} ${_header_FILE})
    endforeach(_current_FILE)

    if(TARGET ${_target_or_source_var})
        target_sources(${_target_or_source_var} PRIVATE ${sources})
    else()
        set(${_target_or_source_var} ${${_target_or_source_var}} ${sources} PARENT_SCOPE)
    endif()

endfunction(KCONFIG_ADD_KCFG_FILES)

function(_convert_to_camelcase ARG_INPUT ARG_OUTPUT)
    string(TOLOWER "${ARG_INPUT}" _input_lc)
    string(REPLACE "_" ";" _input_parts "${_input_lc}")
    set(_result "")
    foreach(_part ${_input_parts})
        string(SUBSTRING "${_part}" 0 1 _first)
        string(TOUPPER "${_first}" _first_uc)
        string(APPEND _result "${_first_uc}")
        string(SUBSTRING "${_part}" 1 -1 _rest)
        string(APPEND _result "${_rest}")
    endforeach()
    set(${ARG_OUTPUT} "${_result}" PARENT_SCOPE)
endfunction()

function(kconfig_target_kcfg_file ARG_TARGET)
    set(_options
        MUTATORS
        SINGLETON
        CUSTOM_ADDITIONS
        NOTIFIERS
        DEFAULT_VALUE_GETTERS
        ITEM_ACCESSORS
        SET_USER_TEXTS
        GLOBAL_ENUMS
        USE_ENUM_TYPES
        FORCE_STRING_FILENAME
        GENERATE_PROPERTIES
        PARENT_IN_CONSTRUCTOR
        GENERATE_MOC
        QML_REGISTRATION
    )
    set(_single_arguments
        FILE
        CLASS_NAME
        VISIBILITY
        HEADER_EXTENSION
        SOURCE_EXTENSION
        NAME_SPACE
        INHERITS
        MEMBER_VARIABLES
        TRANSLATION_SYSTEM
        TRANSLATION_DOMAIN
    )
    set(_multi_arguments
        INCLUDE_FILES
        SOURCE_INCLUDE_FILES
    )
    cmake_parse_arguments(PARSE_ARGV 1 ARG "${_options}" "${_single_arguments}" "${_multi_arguments}")

    if(NOT TARGET ${ARG_TARGET})
        message(FATAL_ERROR "${ARG_TARGET} not found")
    endif()

    if(NOT EXISTS "${ARG_FILE}" AND NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${ARG_FILE}")
        message(FATAL_ERROR "${ARG_FILE} not found")
    endif()

    if("${ARG_CLASS_NAME}" STREQUAL "")
        message(FATAL_ERROR "CLASS_NAME is a required argument")
    endif()

    set(_filepath "${ARG_FILE}")
    get_filename_component(_filename ${ARG_FILE} NAME)
    if(IS_ABSOLUTE ${ARG_FILE})
        set(ARG_FILE ${_filename})
    endif()

    set(_content "")

    foreach(_option ${_options})
        if("${_option}" STREQUAL "GENERATE_MOC")
            continue()
        endif()

        _convert_to_camelcase("${_option}" _entry)
        if(ARG_${_option})
            string(APPEND _content "${_entry}=true\n")
        endif()
    endforeach()

    foreach(_argument ${_single_arguments})
        _convert_to_camelcase("${_argument}" _entry)
        if(ARG_${_argument})
            string(APPEND _content "${_entry}=${ARG_${_argument}}\n")
        endif()
    endforeach()

    foreach(_argument ${_multi_arguments})
        _convert_to_camelcase("${_argument}" _entry)
        if(ARG_${_argument})
            list(JOIN ARG_${_argument} ", " _joined)
            string(APPEND _content "${_entry}=${_joined}\n")
        endif()
    endforeach()

    get_filename_component(_basename ${_filepath} NAME_WE)
    set(_output "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.kcfgc")
    file(WRITE ${_output} "${_content}")

    configure_file(${_filepath} "${CMAKE_CURRENT_BINARY_DIR}/${_filename}" COPYONLY)

    if(${ARG_GENERATE_MOC})
        KCONFIG_ADD_KCFG_FILES(${ARG_TARGET} ${_output} GENERATE_MOC)
    else()
        KCONFIG_ADD_KCFG_FILES(${ARG_TARGET} ${_output})
    endif()
endfunction()
