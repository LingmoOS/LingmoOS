# Copyright 2005-2011 Kitware, Inc.
# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

include(MacroAddFileDependencies)
include(CMakeParseArguments)

function(dtk_add_dbus_interface _sources _interface _relativename)
    get_filename_component(_infile ${_interface} ABSOLUTE)
    get_filename_component(_basepath ${_relativename} DIRECTORY)
    get_filename_component(_basename ${_relativename} NAME)
    set(_header "${CMAKE_CURRENT_BINARY_DIR}/${_relativename}.h")
    set(_impl   "${CMAKE_CURRENT_BINARY_DIR}/${_relativename}.cpp")

    if(${QT_VERSION_MAJOR} EQUAL "5")
        if(_basepath)
           set(_moc "${CMAKE_CURRENT_BINARY_DIR}/${_basepath}/${_basename}.moc")
        else()
           set(_moc "${CMAKE_CURRENT_BINARY_DIR}/${_basename}.moc")
        endif()
    else()
        if(_basepath)
            set(_moc "${CMAKE_CURRENT_BINARY_DIR}/${_basepath}/moc_${_basename}.cpp")
        else()
            set(_moc "${CMAKE_CURRENT_BINARY_DIR}/moc_${_basename}.cpp")
        endif()
    endif()

    get_source_file_property(_nonamespace ${_interface} NO_NAMESPACE)
    if(_nonamespace)
        set(_params -N -m)
    else()
        set(_params -m)
    endif()

    get_source_file_property(_skipincludeannotations ${_interface} SKIP_INCLUDE_ANNOTATIONS)
    if(_skipincludeannotations)
        set(_params ${_params} -S)
    endif()

    get_source_file_property(_classname ${_interface} CLASSNAME)
    if(_classname)
        set(_params ${_params} -c ${_classname})
    endif()

    get_source_file_property(_include ${_interface} INCLUDE)
    if(_include)
        set(_params ${_params} -i ${_include})
    endif()

    add_custom_command(OUTPUT "${_impl}" "${_header}"
        COMMAND ${DTK_XML2CPP} ${_params} -p ${_relativename} ${_infile}
        DEPENDS ${_infile} ${DTK_XML2CPP}
        VERBATIM
    )

    set_source_files_properties("${_impl}" "${_header}" PROPERTIES
        SKIP_AUTOMOC TRUE
        SKIP_AUTOUIC TRUE
    )

    qt_generate_moc("${_header}" "${_moc}")

    list(APPEND ${_sources} "${_impl}" "${_header}")
    set_property(SOURCE "${_impl}" APPEND PROPERTY OBJECT_DEPENDS "${_moc}")
    set(${_sources} ${${_sources}} PARENT_SCOPE)
endfunction()

function(dtk_add_dbus_interfaces _sources)
    foreach(_current_FILE ${ARGN})
        get_filename_component(_infile ${_current_FILE} ABSOLUTE)
        get_filename_component(_basename ${_current_FILE} NAME)
        # get the part before the ".xml" suffix
        string(TOLOWER ${_basename} _basename)
        string(REGEX REPLACE "(.*\\.)?([^\\.]+)\\.xml" "\\2" _basename ${_basename})
        dtk_add_dbus_interface(${_sources} ${_infile} ${_basename}interface)
    endforeach()
    set(${_sources} ${${_sources}} PARENT_SCOPE)
endfunction()
