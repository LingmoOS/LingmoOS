# Try to find DocBook XSL stylesheet
# Once done, it will define:
#
#  DocBookXSL_FOUND - system has the required DocBook XML DTDs
#  DocBookXSL_DIR - the directory containing the stylesheets
#  used to process DocBook XML

# SPDX-FileCopyrightText: 2010 Luigi Toscano <luigi.toscano@tiscali.it>
#
# SPDX-License-Identifier: BSD-3-Clause

include(FeatureSummary)
set_package_properties(DocBookXSL PROPERTIES DESCRIPTION "DocBook XSL"
                       URL "http://docbook.sourceforge.net/release/xsl/current/"
                      )

set (STYLESHEET_PATH_LIST
    ${CMAKE_INSTALL_DATAROOTDIR}/xml/docbook/stylesheet/docbook-xsl
    ${CMAKE_INSTALL_DATAROOTDIR}/xml/docbook/xsl-stylesheets
    ${CMAKE_INSTALL_DATAROOTDIR}/sgml/docbook/xsl-stylesheets
    ${CMAKE_INSTALL_DATAROOTDIR}/xml/docbook/stylesheet/nwalsh/current
    ${CMAKE_INSTALL_DATAROOTDIR}/xml/docbook/stylesheet/nwalsh
    ${CMAKE_INSTALL_DATAROOTDIR}/xsl/docbook
    ${CMAKE_INSTALL_DATAROOTDIR}/xsl/docbook-xsl
    # for building on Mac with docbook-xsl installed by homebrew
    opt/docbook-xsl/docbook-xsl
    # for building with Nix package manager
    xml/xsl/docbook
)

find_path (DocBookXSL_DIR lib/lib.xsl
    PATHS ${CMAKE_SYSTEM_PREFIX_PATH}
    PATH_SUFFIXES ${STYLESHEET_PATH_LIST}
)

if (NOT DocBookXSL_DIR)
    # hacks for systems that put the version in the stylesheet dirs
    set (STYLESHEET_PATH_LIST)
    foreach (STYLESHEET_PREFIX_ITER ${CMAKE_SYSTEM_PREFIX_PATH})
        file(GLOB STYLESHEET_SUFFIX_ITER RELATIVE ${STYLESHEET_PREFIX_ITER}
            ${STYLESHEET_PREFIX_ITER}/share/xml/docbook/xsl-stylesheets-*
        )
        if (STYLESHEET_SUFFIX_ITER)
            list (APPEND STYLESHEET_PATH_LIST ${STYLESHEET_SUFFIX_ITER})
        endif ()
    endforeach ()

    find_path (DocBookXSL_DIR VERSION
        PATHS ${CMAKE_SYSTEM_PREFIX_PATH}
        PATH_SUFFIXES ${STYLESHEET_PATH_LIST}
    )
endif ()


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (DocBookXSL
    REQUIRED_VARS DocBookXSL_DIR
    FOUND_VAR DocBookXSL_FOUND)

#maintain backwards compatibility
set(DOCBOOKXSL_FOUND ${DocBookXSL_FOUND})
set(DOCBOOKXSL_DIR ${DocBookXSL_DIR})

mark_as_advanced (DocBookXSL_DIR)
