# SPDX-FileCopyrightText: 2006-2009 Alexander Neundorf <neundorf@kde.org>
# SPDX-FileCopyrightText: 2006, 2007 Laurent Montel <montel@kde.org>
# SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
#
# SPDX-License-Identifier: BSD-3-Clause
#
#  KDOCTOOLS_CREATE_HANDBOOK( docbookfile [INSTALL_DESTINATION installdest] [SUBDIR subdir])
#   Create the handbook from the docbookfile (using meinproc6)
#   The resulting handbook will be installed to <installdest> when using
#   INSTALL_DESTINATION <installdest>, or to <installdest>/<subdir> if
#   SUBDIR <subdir> is specified.
#
#  KDOCTOOLS_CREATE_MANPAGE( docbookfile section [INSTALL_DESTINATION installdest])
#   Create the manpage for the specified section from the docbookfile (using meinproc6)
#   The resulting manpage will be installed to <installdest> when using
#   INSTALL_DESTINATION <installdest>.
#
#  KDOCTOOLS_INSTALL(podir)
#   Search for docbook files in <podir> and install them to the standard
#   location.
#   This is a convenience function which relies on all docbooks being kept in
#   <podir>/<lang>/docs/<project>, where <lang> is the language the docbooks
#   for <project> are written in.
#
#   Within this directory, files ending with .[0-9].docbook are installed using
#   KDOCTOOLS_CREATE_MANPAGE, other .docbook files are installed using
#   KDOCTOOLS_CREATE_HANDBOOK if index.docbook is available.
#
#   For example, given the following directory structure:
#
#    po/
#      fr/
#        docs/
#          foo/
#            kioworker6/
#              fooworker/
#                index.docbook
#            footool.1.docbook
#            footool.conf.5.docbook
#            index.docbook
#
#   KDOCTOOLS_INSTALL(po) does the following:
#   - Create man pages from footool.1.docbook and footool.conf.5.docbook,
#     install them in ${KDE_INSTALL_MANDIR}/fr
#   - Create handbooks from index.docbook files, install the one from the
#     fooworker/ directory in ${KDE_INSTALL_DOCBUNDLEDIR}/fr/kioworker6/fooworker
#     and the one from the docs/ directory in ${KDE_INSTALL_DOCBUNDLEDIR}/fr
#
#   If ${KDE_INSTALL_DOCBUNDLEDIR} is not set, share/doc/HTML is used instead.
#   If ${KDE_INSTALL_MANDIR} is not set, share/man/<lang> is used instead.
#
#  KDOCTOOLS_MEINPROC_EXECUTABLE - the meinproc6 executable
#
#  KDOCTOOLS_SERIALIZE_TOOL - wrapper to serialize potentially resource-intensive commands during
#                      parallel builds (set to 'icecc' when using icecream)
#
# The following variables are defined for the various tools required to
# compile KDE software:
#
#  KDOCTOOLS_MEINPROC_EXECUTABLE - the meinproc6 executable
#

# Resetting policies for code in this module scope
# Needed for IN_LIST usage ( CMP0057 )
cmake_policy(VERSION 3.16)

set(KDOCTOOLS_SERIALIZE_TOOL "" CACHE STRING "Tool to serialize resource-intensive commands in parallel builds")
set(KDOCTOOLS_MEINPROC_EXECUTABLE "KF6::meinproc6")

if(KDOCTOOLS_SERIALIZE_TOOL)
    # parallel build with many meinproc invocations can consume a huge amount of memory
    set(KDOCTOOLS_MEINPROC_EXECUTABLE ${KDOCTOOLS_SERIALIZE_TOOL} ${KDOCTOOLS_MEINPROC_EXECUTABLE})
endif(KDOCTOOLS_SERIALIZE_TOOL)

function(_kdoctools_create_target_name out in)
    file(RELATIVE_PATH in "${CMAKE_BINARY_DIR}" "${in}")
    string(REGEX REPLACE "[^0-9a-zA-Z]+" "-" tmp "${in}")
    set(${out} ${tmp} PARENT_SCOPE)
endfunction()

function (kdoctools_create_handbook docbook)
    # Parse arguments
    set(options)
    set(oneValueArgs INSTALL_DESTINATION SUBDIR)
    set(multiValueArgs)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT DEFINED ARGS_SUBDIR)
        message(FATAL_ERROR "SUBDIR needs to be defined when calling kdoctools_create_handbook")
    endif()
    if (INSTALL_DESTINATION IN_LIST ARGS_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "INSTALL_DESTINATION got an empty value set when calling kdoctools_create_handbook")
    endif()

    # Init vars
    get_filename_component(docbook ${docbook} ABSOLUTE)
    file(RELATIVE_PATH src_doc ${CMAKE_CURRENT_SOURCE_DIR} ${docbook})
    get_filename_component(src_dir ${src_doc} DIRECTORY)
    set(build_dir ${CMAKE_CURRENT_BINARY_DIR}/${src_dir})
    set(build_doc ${build_dir}/index.cache.bz2)
    set(build_html ${build_dir}/index.html)

    # current directory is the docbook directory, but if this is empty, the
    # globs which finds the docbooks and the images will be empty too as
    # they will expand into "/*.docbook" and "/*.png"
    if (NOT src_dir)
        set(src_dir ".")
    endif ()

    # Create some place to store our files
    file(MAKE_DIRECTORY ${build_dir})

    #Bootstrap
    if (_kdoctoolsBootStrapping OR _kdoctoolsTestsRunning)
        set(_bootstrapOption "--srcdir=${KDocTools_BINARY_DIR}/src")
    elseif (CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(_bootstrapOption "--srcdir=${KDOCTOOLS_DATA_INSTALL_DIR}/kdoctools")
    else ()
        set(_bootstrapOption)
    endif ()
    set(_ssheet "${KDOCTOOLS_CUSTOMIZATION_DIR}/kde-chunk.xsl")

    file(GLOB candidate_src_docs ${src_dir}/*.docbook)

    set(src_docs)
    foreach (src_single_doc ${candidate_src_docs})
        # Exclude manpages
        get_filename_component(src_single_doc_name ${src_single_doc} NAME)
        if (NOT src_single_doc_name MATCHES "^man-.+\\.docbook$")
            list(APPEND src_docs ${src_single_doc})
        endif()
    endforeach()

    add_custom_command(OUTPUT ${build_doc}
        COMMAND ${KDOCTOOLS_MEINPROC_EXECUTABLE} --check ${_bootstrapOption} --cache ${build_doc} ${src_doc}
        DEPENDS ${src_docs} ${_ssheet}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )

    _kdoctools_create_target_name(_targ ${build_doc})
    add_custom_target(${_targ} ALL DEPENDS ${build_doc})

    if(KDOCTOOLS_ENABLE_HTMLHANDBOOK)
        add_custom_command(OUTPUT ${build_html}
            COMMAND ${KDOCTOOLS_MEINPROC_EXECUTABLE} --check ${_bootstrapOption} -o ${build_html} ${src_doc}
            DEPENDS ${src_doc} ${_ssheet}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        )

        _kdoctools_create_target_name(_targ_html ${build_html})
        add_custom_target(${_targ_html} ALL DEPENDS ${build_html})
    endif(KDOCTOOLS_ENABLE_HTMLHANDBOOK)

    set(installDest "${ARGS_INSTALL_DESTINATION}")
    if(installDest)
        set(subdir "${ARGS_SUBDIR}")
        file(GLOB images ${src_dir}/*.png)
        install(FILES ${build_doc} ${src_docs} ${images} DESTINATION ${installDest}/${subdir})
        if(KDOCTOOLS_ENABLE_HTMLHANDBOOK)
            install(FILES ${build_html} DESTINATION ${installDest}/${subdir})
        endif(KDOCTOOLS_ENABLE_HTMLHANDBOOK)
    endif()

endfunction()


function (kdoctools_create_manpage docbook section)
    # Parse arguments
    set(options)
    set(oneValueArgs INSTALL_DESTINATION)
    set(multiValueArgs)
    cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if (INSTALL_DESTINATION IN_LIST ARGS_KEYWORDS_MISSING_VALUES)
        message(FATAL_ERROR "INSTALL_DESTINATION got an empty value set when calling kdoctools_create_manpage")
    endif()

    # Init vars
    get_filename_component(docbook ${docbook} ABSOLUTE)
    file(RELATIVE_PATH src_doc ${CMAKE_CURRENT_SOURCE_DIR} ${docbook})
    get_filename_component(src_dir ${src_doc} DIRECTORY)
    set(build_dir ${CMAKE_CURRENT_BINARY_DIR}/${src_dir})

    get_filename_component(name ${src_doc} NAME)
    string(REGEX REPLACE "^man-(.*)\\.${section}\\.docbook$" "\\1" name ${name})
    set(build_doc ${build_dir}/${name}.${section})

    # Create some place to store our files
    file(MAKE_DIRECTORY ${build_dir})

    #Bootstrap
    if (_kdoctoolsBootStrapping)
        set(_bootstrapOption "--srcdir=${KDocTools_BINARY_DIR}/src")
        set(_extraDependency "docbookl10nhelper")
    elseif (_kdoctoolsTestsRunning)
        set(_bootstrapOption "--srcdir=${KDocTools_BINARY_DIR}/src")
        set(_extraDependency)
    elseif (CMAKE_CROSSCOMPILING AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
        set(_bootstrapOption "--srcdir=${KDOCTOOLS_DATA_INSTALL_DIR}/kdoctools")
        set(_extraDependency)
    else ()
        set(_bootstrapOption)
        set(_extraDependency)
    endif ()
    set(_ssheet "${KDOCTOOLS_CUSTOMIZATION_DIR}/kde-include-man.xsl")

    add_custom_command(OUTPUT ${build_doc}
        COMMAND ${KDOCTOOLS_MEINPROC_EXECUTABLE} --stylesheet ${_ssheet} --check ${_bootstrapOption} ${CMAKE_CURRENT_SOURCE_DIR}/${src_doc}
        DEPENDS ${src_doc} ${_ssheet} ${_extraDependency}
        WORKING_DIRECTORY ${build_dir}
    )

    _kdoctools_create_target_name(_targ ${build_doc})
    add_custom_target(${_targ} ALL DEPENDS "${build_doc}")

    if(ARGS_INSTALL_DESTINATION)
        install(FILES ${build_doc} DESTINATION ${ARGS_INSTALL_DESTINATION}/man${section})
    endif()
endfunction()


function(kdoctools_install podir)
    file(GLOB lang_dirs "${podir}/*")
    if (NOT KDE_INSTALL_MANDIR)
        if (MAN_INSTALL_DIR) # TODO KF6: deprecated, remove
            set(KDE_INSTALL_MANDIR ${MAN_INSTALL_DIR})
        else()
            set(KDE_INSTALL_MANDIR share/man)
        endif()
    endif()
    if (NOT KDE_INSTALL_DOCBUNDLEDIR)
        if (HTML_INSTALL_DIR) # TODO KF6: deprecated, remove
            set(KDE_INSTALL_DOCBUNDLEDIR ${HTML_INSTALL_DIR})
        else()
            set(KDE_INSTALL_DOCBUNDLEDIR share/doc/HTML)
        endif()
    endif()
    foreach(lang_dir ${lang_dirs})
        get_filename_component(lang ${lang_dir} NAME)

        file(GLOB_RECURSE docbooks RELATIVE "${lang_dir}" "${lang_dir}/docs/*.docbook")
        foreach(docbook ${docbooks})
            string(REGEX MATCH "\\.([0-9])\\.docbook" match ${docbook})
            if (match)
                kdoctools_create_manpage("${lang_dir}/${docbook}" ${CMAKE_MATCH_1}
                    INSTALL_DESTINATION ${KDE_INSTALL_MANDIR}/${lang}
                )
            else()
                string(REGEX MATCH "^docs/(.*)/index.docbook" match ${docbook})
                if (match)
                    kdoctools_create_handbook("${lang_dir}/${docbook}"
                        INSTALL_DESTINATION ${KDE_INSTALL_DOCBUNDLEDIR}/${lang}
                        SUBDIR ${CMAKE_MATCH_1}
                    )
                endif()
            endif()
        endforeach()
    endforeach()
endfunction()
