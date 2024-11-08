cmake_minimum_required(VERSION 3.14)
project(panel-ukcc-plugin)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(PANEL_UKCC_PLUGIN_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

find_package(PkgConfig REQUIRED)
find_package(lingmo-quick COMPONENTS framework core)
set(UKCC_PLUGIN_PC_PKGS lingmosdk-qtwidgets)

foreach(PC_LIB IN ITEMS ${UKCC_PLUGIN_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND UKCC_PLUGIN_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

file(GLOB UKCC_PLUGIN_TS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts)
qt5_create_translation(UKCC_PLUGIN_QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${UKCC_PLUGIN_TS_FILES})

set(UKCC_PLUGIN_SRC
        switch-button.cpp
        switch-button.h
        panel-ukcc-plugin.cpp
        panel-ukcc-plugin.h
        ../src/general-config-define.h
)
qt5_generate_repc(UKCC_PLUGIN_SRC ../src/remote-config.rep REPLICA)
add_library(panel-ukcc-plugin MODULE
        ${UKCC_PLUGIN_SRC}
        ${UKCC_PLUGIN_QM_FILES}
)
set(TRANSLATIONS_INSTALL_PATH /usr/share/lingmo-panel/panel-ukcc-plugin/translations/)
target_compile_definitions(panel-ukcc-plugin PRIVATE
        VERSION="${PANEL_UKCC_PLUGIN_VERSION}"
        TRANSLATIONS_INSTALL_PATH="${TRANSLATIONS_INSTALL_PATH}"
)
target_link_libraries(panel-ukcc-plugin PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::RemoteObjects
        ukcc
        ${UKCC_PLUGIN_PC_PKGS}
        lingmo-quick::framework
        lingmo-quick::core
)

install(TARGETS panel-ukcc-plugin
        DESTINATION /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/lingmo-control-center)
install(FILES
        ${UKCC_PLUGIN_TS_FILES}
        ${UKCC_PLUGIN_QM_FILES}
        DESTINATION ${TRANSLATIONS_INSTALL_PATH})
