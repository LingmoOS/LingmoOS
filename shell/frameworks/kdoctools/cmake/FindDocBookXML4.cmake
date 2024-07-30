# Try to find DocBook XML 4.x DTD.
# By default it will find version 4.5. A different version can be specified
# as parameter for find_package().
# Once done, it will define:
#
#  DocBookXML4_FOUND - system has the requested DocBook4 XML DTDs
#  DocBookXML4_DTD_VERSION - the version of requested DocBook4
#     XML DTD
#  DocBookXML4_DTD_DIR - the directory containing the definition of
#     the DocBook4 XML

# SPDX-FileCopyrightText: 2010, 2014 Luigi Toscano <luigi.toscano@tiscali.it>
#
# SPDX-License-Identifier: BSD-3-Clause

if (NOT DocBookXML_FIND_VERSION)
     set(DocBookXML_FIND_VERSION "4.5")
endif ()

set (DocBookXML4_DTD_VERSION ${DocBookXML_FIND_VERSION}
     CACHE INTERNAL "Required version of DocBook4 XML DTDs")

include(FeatureSummary)
set_package_properties(DocBookXML4 PROPERTIES DESCRIPTION "DocBook XML 4"
                       URL "http://www.oasis-open.org/docbook/xml/${DocBookXML4_DTD_VERSION}"
                      )

function (locate_version version found_dir)

    set (DTD_PATH_LIST
        ${CMAKE_INSTALL_DATAROOTDIR}/xml/docbook/schema/dtd/${version}
        ${CMAKE_INSTALL_DATAROOTDIR}/xml/docbook/xml-dtd-${version}
        ${CMAKE_INSTALL_DATAROOTDIR}/sgml/docbook/xml-dtd-${version}
        ${CMAKE_INSTALL_DATAROOTDIR}/xml/docbook/${version}
        # for building on Mac with docbook installed by homebrew
        opt/docbook/docbook/xml/${version}
        # for building with Nix package manager
        xml/dtd/docbook
    )

    find_path (searched_dir docbookx.dtd
        PATHS ${CMAKE_SYSTEM_PREFIX_PATH}
        PATH_SUFFIXES ${DTD_PATH_LIST}
    )

    if (NOT searched_dir)
        # hacks for systems that use the package version in the DTD dirs,
        # e.g. Fedora, OpenSolaris
        set (DTD_PATH_LIST)
        foreach (DTD_PREFIX_ITER ${CMAKE_SYSTEM_PREFIX_PATH})
            file(GLOB DTD_SUFFIX_ITER RELATIVE ${DTD_PREFIX_ITER}
                ${DTD_PREFIX_ITER}/share/sgml/docbook/xml-dtd-${version}-*
            )
            if (DTD_SUFFIX_ITER)
                list (APPEND DTD_PATH_LIST ${DTD_SUFFIX_ITER})
            endif ()
        endforeach ()

        find_path (searched_dir docbookx.dtd
            PATHS ${CMAKE_SYSTEM_PREFIX_PATH}
            PATH_SUFFIXES ${DTD_PATH_LIST}
        )
    endif ()
    if (searched_dir)
        set (${found_dir} ${searched_dir} PARENT_SCOPE)
    else()
        message(WARNING "${found_dir}: Could not find docbookx.dtd in ${CMAKE_SYSTEM_PREFIX_PATH} with suffixes ${DTD_PATH_LIST}")
    endif()
endfunction()


locate_version (${DocBookXML4_DTD_VERSION} DocBookXML4_DTD_DIR)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args (DocBookXML4
    REQUIRED_VARS DocBookXML4_DTD_DIR DocBookXML4_DTD_VERSION
    FOUND_VAR DocBookXML4_FOUND)

mark_as_advanced (DocBookXML4_DTD_DIR DocBookXML4_DTD_VERSION)
