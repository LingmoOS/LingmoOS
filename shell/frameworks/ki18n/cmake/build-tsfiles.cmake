# SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

file(GLOB_RECURSE ts_files RELATIVE ${PO_DIR} ${PO_DIR}/**/scripts/*)
foreach(ts_file ${ts_files})
    if(ts_file MATCHES "\\.svn")
        continue()
    endif()

    get_filename_component(subpath ${ts_file} DIRECTORY)
    string(REPLACE "scripts" "LC_SCRIPTS" subpath ${subpath})

    message(STATUS "copying... ${PO_DIR}/${ts_file} DESTINATION ${COPY_TO}/${subpath}" )
    file(COPY ${PO_DIR}/${ts_file} DESTINATION ${COPY_TO}/${subpath})
endforeach()


include(ProcessorCount)
ProcessorCount(numberOfProcesses)

set(i 0)
set(commands)

function(_processCommands)
    if(NOT commands)
        return()
    endif()

    execute_process(
        ${commands}
        RESULT_VARIABLE code
    )
    if(code)
        message(FATAL_ERROR "failed generating: ${PO_DIR}")
    endif()
endfunction()

file(GLOB_RECURSE pmap_files RELATIVE ${PO_DIR} "${PO_DIR}/**.pmap")
foreach(pmap_file ${pmap_files})
    get_filename_component(pmap_basename ${pmap_file} NAME)
    get_filename_component(subpath ${pmap_file} DIRECTORY)
    string(REPLACE "scripts" "LC_SCRIPTS" subpath ${subpath})
    set(pmapc_file "${COPY_TO}/${subpath}/${pmap_basename}c")

    if (EXISTS ${pmapc_file} AND ${pmapc_file} IS_NEWER_THAN ${pmap_file})
        continue()
    endif()

    message(STATUS "building... ${pmap_file} to ${pmapc_file}" )
    list(APPEND commands
        COMMAND ${Python3_EXECUTABLE}
            -B
            ${_ki18n_pmap_compile_script}
            ${PO_DIR}/${pmap_file}
            ${pmapc_file}
    )
    math(EXPR i "${i}+1")
    if (i EQUAL ${numberOfProcesses})
        _processCommands()
        set(i 0)
    endif()
endforeach()

_processCommands()
