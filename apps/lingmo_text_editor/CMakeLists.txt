# SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
# SPDX-FileCopyrightText: 2026 LingmoOS Contributors
# SPDX-License-Identifier: GPL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(lingmo-text-editor VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc)
# PCRE2 runs regular-expression find.
pkg_check_modules(PCRE2 REQUIRED IMPORTED_TARGET libpcre2-8)

add_library(editor-core STATIC
    src/document.cpp
    src/fileoptions.cpp
    src/recentfiles.cpp
    src/regexp.cpp
    src/searcher.cpp
    src/session.cpp
    src/syntax.cpp
    src/textbuffer.cpp
    src/textcodec.cpp
    src/undostack.cpp
    src/unicode.cpp
)
target_include_directories(editor-core PUBLIC src)
target_link_libraries(editor-core PUBLIC Lingmo::SDKBase PRIVATE PkgConfig::ICU PkgConfig::PCRE2)

add_library(editor-ui STATIC
    src/application.cpp
    src/codelayout.cpp
    src/colors.cpp
    src/controls.cpp
    src/dialogs.cpp
    src/editorpage.cpp
    src/editorsettings.cpp
    src/editorview.cpp
    src/editorwindow.cpp
    src/pagebars.cpp
    src/syntaxtheme.cpp
)
target_include_directories(editor-ui PUBLIC src)
target_link_libraries(editor-ui PUBLIC editor-core Lingmo::SDK PRIVATE PkgConfig::ICU)

add_executable(${PROJECT_NAME} src/main.cpp)
target_compile_definitions(${PROJECT_NAME} PRIVATE TEXT_EDITOR_VERSION="${PROJECT_VERSION}")
target_link_libraries(${PROJECT_NAME} PRIVATE editor-ui)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

lingmo_add_translations(lingmo-text-editor SOURCES src DESKTOP_FILES lingmo-text-editor.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-text-editor.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
