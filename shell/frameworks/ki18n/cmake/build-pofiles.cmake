# SPDX-FileCopyrightText: 2017 Aleix Pol Gonzalez <aleixpol@kde.org>
# SPDX-FileCopyrightText: 2017 Harald Sitter <sitter@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause

file(GLOB_RECURSE pofiles RELATIVE "${PO_DIR}" "${PO_DIR}/**.po")

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
        message(FATAL_ERROR "failed generating ${PO_DIR}")
    endif()
endfunction()

foreach(pofile IN LISTS pofiles)
    get_filename_component(name ${pofile} NAME)
    # Regex the basename, cmake only allows stripping the longest extension, we
    # need the shortest or we'll screw up "org.kde.plasma.kittens.po"
    # https://bugs.kde.org/show_bug.cgi?id=379116
    string(REGEX REPLACE "^(.+)(\\.[^.]+)$" "\\1" name ${name})
    get_filename_component(langdir ${pofile} DIRECTORY)
    set(dest ${COPY_TO}/${langdir}/LC_MESSAGES)
    file(MAKE_DIRECTORY ${dest})

    if (EXISTS ${dest}/${name}.mo AND ${dest}/${name}.mo IS_NEWER_THAN ${PO_DIR}/${pofile})
        continue()
    endif()

    list(APPEND commands COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} -o ${dest}/${name}.mo ${PO_DIR}/${pofile})
    math(EXPR i "${i}+1")
    if(i EQUAL ${numberOfProcesses})
        _processCommands()
        set(i 0)
        set(commands)
    endif()
endforeach()

_processCommands()
