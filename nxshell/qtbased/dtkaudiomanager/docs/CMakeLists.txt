# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#${CMAKE_INSTALL_DATAROOTDIR}/qt5/doc

set (QCH_INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qt5/doc CACHE STRING "QCH install location")
set (DOXYGEN_GENERATE_HTML "YES" CACHE STRING "Doxygen HTML output")
set (DOXYGEN_GENERATE_XML "NO" CACHE STRING "Doxygen XML output")
set (DOXYGEN_GENERATE_QHP "YES" CACHE STRING "Doxygen QHP output")
set (DOXYGEN_FILE_PATTERNS *.cpp *.h *.md *.zh_CN.dox CACHE STRING "Doxygen File Patterns")
set(DOXYGEN_EXCLUDE_PATTERNS *_p.h)
set (DOXYGEN_PROJECT_NUMBER ${CMAKE_PROJECT_VERSION} CACHE STRING "") # Should be the same as this project is using.
set (DOXYGEN_EXTRACT_STATIC YES)
set (DOXYGEN_OUTPUT_LANGUAGE "Chinese")
set (DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs/)
set (DOXYGEN_QHG_LOCATION "qhelpgenerator")
set (DOXYGEN_QHP_NAMESPACE "org.deepin.dde.${CMAKE_PROJECT_NAME}")
set (DOXYGEN_QCH_FILE "${CMAKE_PROJECT_NAME}.qch")
set (DOXYGEN_QHP_VIRTUAL_FOLDER ${CMAKE_PROJECT_NAME})
set (DOXYGEN_HTML_EXTRA_STYLESHEET "" CACHE STRING "Doxygen custom stylesheet for HTML output")
set (DOXYGEN_TAGFILES "qtcore.tags=qthelp://doc.qt.io/qt-5/" CACHE STRING "Doxygen tag files")
set (DOXYGEN_MACRO_EXPANSION "YES")
set (DOXYGEN_EXPAND_ONLY_PREDEF "YES")
set (DOXYGEN_PREDEFINED
    "DAUDIOMANAGER_BEGIN_NAMESPACE=namespace Dtk { namespace AudioManager {"
    "DAUDIOMANAGER_END_NAMESPACE=}}"
    "DAUDIOMANAGER_USE_NAMESPACE=using namespace Dtk::AudioManager;"
)

if(BUILD_THEME)
set (DOXYGEN_HTML_EXTRA_STYLESHEET "../doxygentheme/doxygen-awesome.css"
                                "../doxygentheme/doxygen-awesome-sidebar-only.css"
                                "../doxygentheme/doxygen-awesome-sidebar-only-darkmode-toggle.css"
                                )
set (DOXYGEN_HTML_EXTRA_FILES "../doxygentheme/doxygen-awesome-darkmode-toggle.js"
                            "../doxygentheme/doxygen-awesome-fragment-copy-button.js"
                            "../doxygentheme/doxygen-awesome-paragraph-link.js"
                            "../doxygentheme/doxygen-awesome-interactive-toc.js"
                            )
set (DOXYGEN_GENERATE_TREEVIEW "YES")
set (DOXYGEN_DISABLE_INDEX "NO")
set (DOXYGEN_FULL_SIDEBAR "NO")
set (DOXYGEN_HTML_HEADER "../doxygentheme/header.html")
endif()

doxygen_add_docs (doxygen
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../docs
    ALL
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../
    COMMENT "Generate documentation via Doxygen"
)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/html/${CMAKE_PROJECT_NAME}.qch DESTINATION ${QCH_INSTALL_DESTINATION})
