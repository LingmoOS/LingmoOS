# SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set(QCH_INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qt${QT_VERSION_MAJOR}/doc CACHE STRING "QCH install location")
set(DOXYGEN_GENERATE_HTML "YES" CACHE STRING "Doxygen HTML output")
set(DOXYGEN_GENERATE_XML "NO" CACHE STRING "Doxygen XML output")
set(DOXYGEN_GENERATE_QHP "YES" CACHE STRING "Doxygen QHP output")
set(DOXYGEN_FILE_PATTERNS *.cpp *.h *.md *.zh_CN.dox CACHE STRING "Doxygen File Patterns")
set(DOXYGEN_PROJECT_NUMBER ${CMAKE_PROJECT_VERSION} CACHE STRING "") # Should be the same as this project is using.
set(DOXYGEN_EXTRACT_STATIC YES)
set(DOXYGEN_OUTPUT_LANGUAGE "Chinese")
set(DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs/)
set(DOXYGEN_QHG_LOCATION "qhelpgenerator")
set(DOXYGEN_QHP_NAMESPACE "org.deepin.dtk.networkmanager")
set(DOXYGEN_QCH_FILE "${LIB_NAME}.qch")
set(DOXYGEN_QHP_VIRTUAL_FOLDER ${CMAKE_PROJECT_NAME})
set(DOXYGEN_HTML_EXTRA_STYLESHEET "" CACHE STRING "Doxygen custom stylesheet for HTML output")
set(DOXYGEN_TAGFILES "qtcore.tags=qthelp://org.qt-project.qtcore/qtcore/" CACHE STRING "Doxygen tag files")
set(DOXYGEN_MACRO_EXPANSION "YES")
set(DOXYGEN_EXPAND_ONLY_PREDEF "YES")
set(DOXYGEN_PREDEFINED
    "DNETWORKMANAGER_BEGIN_NAMESPACE=namespace Dtk { namespace NetworkManager {"
    "DNETWORKMANAGER_END_NAMESPACE=}}"
    "DNETWORKMANAGER_USE_NAMESPACE=using namespace Dtk::NetworkManager;"
    "DTK_DNETWORKMANAGER_NAMESPACE=Dtk::NetworkManager"
)

doxygen_add_docs (doxygen
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/docs
    ALL
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generate documentation via Doxygen"
)

install (FILES ${CMAKE_CURRENT_BINARY_DIR}/html/${LIB_NAME}.qch DESTINATION ${QCH_INSTALL_DESTINATION})
