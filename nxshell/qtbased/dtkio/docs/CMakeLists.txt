# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

set (QCH_INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qt5/doc CACHE STRING "QCH install location")
set (DOXYGEN_GENERATE_HTML "YES" CACHE STRING "Doxygen HTML output")
set (DOXYGEN_GENERATE_XML "NO" CACHE STRING "Doxygen XML output")
set (DOXYGEN_GENERATE_QHP "YES" CACHE STRING "Doxygen QHP output")
set (DOXYGEN_FILE_PATTERNS *.cpp *.h *.md *.zh_CN.dox CACHE STRING "Doxygen File Patterns")
set (DOXYGEN_PROJECT_NUMBER ${CMAKE_PROJECT_VERSION} CACHE STRING "") # Should be the same as this project is using.
set (DOXYGEN_EXTRACT_STATIC YES)
set (DOXYGEN_OUTPUT_LANGUAGE "Chinese")
set (DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs/)
set (DOXYGEN_QHG_LOCATION "qhelpgenerator")
set (DOXYGEN_QHP_NAMESPACE "org.deepin.dtk.io")
set (DOXYGEN_QCH_FILE "${PROJECT_NAME}.qch")
set (DOXYGEN_QHP_VIRTUAL_FOLDER ${PROJECT_NAME})
set (DOXYGEN_HTML_EXTRA_STYLESHEET "" CACHE STRING "Doxygen custom stylesheet for HTML output")
set (DOXYGEN_TAGFILES "qtcore.tags=qthelp://doc.qt.io/qt-5/" CACHE STRING "Doxygen tag files")
set (DOXYGEN_MACRO_EXPANSION "YES")
set (DOXYGEN_EXPAND_ONLY_PREDEF "YES")
set (DOXYGEN_PREDEFINED
    "DBURN_BEGIN_NAMESPACE=namespace Dtk { namespace Burn {"
    "DBURN_END_NAMESPACE=}}"
    "DBURN_USE_NAMESPACE=using namespace Dtk::Burn;"
    "DIO_BEGIN_NAMESPACE=namespace Dtk { namespace IO {"
    "DIO_END_NAMESPACE=}}"
    "DIO_USE_NAMESPACE=using namespace Dtk::IO;"
    "DMOUNT_BEGIN_NAMESPACE=namespace Dtk { namespace Mount {"
    "DMOUNT_END_NAMESPACE=}}"
    "DMOUNT_USE_NAMESPACE=using namespace Dtk::Mount;"
    "DSEARCH_BEGIN_NAMESPACE=namespace Dtk { namespace Search {"
    "DSEARCH_END_NAMESPACE=}}"
    "DSEARCH_USE_NAMESPACE=using namespace Dtk::Search;"
    "DCOMPRESSOR_BEGIN_NAMESPACE=namespace Dtk { namespace Compressor {"
    "DCOMPRESSOR_END_NAMESPACE=}}"
    "DCOMPRESSOR_USE_NAMESPACE=using namespace Dtk::Compressor;"
)

doxygen_add_docs (doxygen
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/docs
    ALL
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generate documentation via Doxygen"
)

install (FILES ${PROJECT_BINARY_DIR}/docs/html/${PROJECT_NAME}.qch DESTINATION ${QCH_INSTALL_DESTINATION})
