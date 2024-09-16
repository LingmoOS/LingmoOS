# SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: LGPL-3.0-or-later

find_package(Doxygen REQUIRED)
set(QCH_INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/qt${QT_VERSION_MAJOR}/doc CACHE STRING "QCH install location")
set(DOXYGEN_GENERATE_HTML YES CACHE STRING "Doxygen HTML output")
set(DOXYGEN_GENERATE_XML YES CACHE STRING "Doxygen XML output")
set(DOXYGEN_GENERATE_QHP YES CACHE STRING "Doxygen QHP output")
set (DOXYGEN_FILE_PATTERNS *.cpp *.h *.md *.zh_CN.dox CACHE STRING "Doxygen File Patterns")
set (DOXYGEN_PROJECT_NUMBER ${CMAKE_PROJECT_VERSION} CACHE STRING "") # Should be the same as this project is using.
set (DOXYGEN_EXTRACT_STATIC YES)
set (DOXYGEN_OUTPUT_LANGUAGE "Chinese")

if("${QT_VERSION_MAJOR}" STREQUAL "5")
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Help)
else()
    find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS ToolsTools)
endif()

get_target_property(_qhelpgenerator_location Qt${QT_VERSION_MAJOR}::qhelpgenerator IMPORTED_LOCATION)

if("${_qhelpgenerator_location}" STREQUAL "")
    set(_qhelpgenerator_location "qhelpgenerator")
endif()

set(DOXYGEN_QHG_LOCATION ${_qhelpgenerator_location} CACHE STRING "Doxygen QHG path")

set(DOXYGEN_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/docs/)
set (DOXYGEN_QHP_NAMESPACE "org.deepin.dtk.systemsettings")
set (DOXYGEN_QCH_FILE "${LIB_NAME}.qch")
set (DOXYGEN_QHP_VIRTUAL_FOLDER ${LIB_NAME})
set (DOXYGEN_HTML_EXTRA_STYLESHEET "" CACHE STRING "Doxygen custom stylesheet for HTML output")
set(DOXYGEN_TAGFILES "qtcore.tags=qthelp://doc.qt.io/qt-${QT_VERSION_MAJOR}/" CACHE STRING "Doxygen tag files")
set (DOXYGEN_MACRO_EXPANSION "YES")
set (DOXYGEN_EXPAND_ONLY_PREDEF "YES")
set (DOXYGEN_PREDEFINED
    "DACCOUNTS_BEGIN_NAMESPACE=namespace Dtk { namespace Accounts {"
    "DACCOUNTS_END_NAMESPACE=}}"
    "DACCOUNTS_USE_NAMESPACE=using namespace Dtk::Accounts;"
    "DLOGIN_BEGIN_NAMESPACE=namespace Dtk { namespace Login {"
    "DLOGIN_END_NAMESPACE=}}"
    "DLOGIN_USE_NAMESPACE=using namespace Dtk::Login;"
    "DPOWER_BEGIN_NAMESPACE=namespace Dtk { namespace Power {"
    "DPOWER_END_NAMESPACE=}}"
    "DPOWER_USE_NAMESPACE=using namespace Dtk::Power;"
    "DRFMGR_BEGIN_NAMESPACE=namespace Dtk { namespace RfMgr {"
    "DRFMGR_END_NAMESPACE=}}"
    "DRFMGR_USE_NAMESPACE=using namespace Dtk::RfMgr;"
    "DSYSTEMTIME_BEGIN_NAMESPACE=namespace Dtk { namespace SystemTime {"
    "DSYSTEMTIME_END_NAMESPACE=}}"
    "DTIME_USE_NAMESPACE=using namespace Dtk::SystemTime;"
)

doxygen_add_docs (doxygen
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/docs
    ALL
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    COMMENT "Generate documentation via Doxygen"
)

install (FILES ${PROJECT_BINARY_DIR}/docs/html/${LIB_NAME}.qch DESTINATION ${QCH_INSTALL_DESTINATION})
