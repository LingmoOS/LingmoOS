cmake_minimum_required(VERSION 3.16)

project(lingmo-panel)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick Widgets DBus LinguistTools RemoteObjects REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick Widgets DBus LinguistTools RemoteObjects REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(lingmo-quick COMPONENTS core framework platform REQUIRED)

add_subdirectory(3rd-parties/qtsingleapplication)
include_directories(3rd-parties/qtsingleapplication/src)

include_directories(src)
include_directories(src/view)

file(GLOB TS_FILES translations/*.ts)
set(QRC_FILES qml/qml.qrc)

set(LINGMO_PANEL_EXTERNAL_LIBS "")
set(LINGMO_PANEL_PC_PKGS gsettings-qt lingmosdk-waylandhelper lingmosdk-datacollect)

foreach(PC_LIB IN ITEMS ${LINGMO_PANEL_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND LINGMO_PANEL_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

set(PROJECT_SOURCES
        src/main.cpp
        src/shell.cpp
        src/view/panel.cpp
        src/log-utils.cpp
        src/remote-config.cpp
        src/remote-config.h
        src/general-config-define.h
        src/screens-manager.cpp
        src/screens-manager.h
        src/view/widget-model.cpp
        src/view/widget-model.h
        src/view/common.h
        src/data-collector.h
        src/data-collector.cpp
)
qt5_generate_repc(PROJECT_SOURCES src/remote-config.rep SOURCE)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/qml ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    qt_add_executable(${PROJECT_NAME}
            MANUAL_FINALIZATION
            ${PROJECT_SOURCES}
            ${QRC_FILES}
            ${QM_FILES}
    )

else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/qml ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
    add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
            ${QRC_FILES}
            ${QM_FILES}
    )

endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:PANEL_DISABLE_LOG_FILE>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:QT_QML_DEBUG>)
set(PANEL_TRANSLATION_DIR "/usr/share/lingmo-panel/translations/")
add_compile_definitions(PANEL_TRANSLATION_DIR="${PANEL_TRANSLATION_DIR}")

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::RemoteObjects
        KF5::WindowSystem
        qtsingleapplication
        ${LINGMO_PANEL_EXTERNAL_LIBS}
        lingmo-quick::framework
        lingmo-quick::core
        lingmo-quick::platform
)

add_subdirectory(ukcc-plugin)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "/usr/bin")
install(FILES ${QM_FILES} DESTINATION "${PANEL_TRANSLATION_DIR}")
install(FILES lingmo-panel.desktop DESTINATION /etc/xdg/autostart)
install(FILES data/org.lingmo.panel.settings.gschema.xml DESTINATION "/usr/share/glib-2.0/schemas")
