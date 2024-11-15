cmake_minimum_required(VERSION 3.5)

# set(PROJECT_NAME lingmo-control-center)
set(PROJECT_NAME lingmo-settings)
project(${PROJECT_NAME})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-fcoroutines ${CMAKE_CXX_FLAGS}")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake;${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# set(QT Core Gui Widgets Network WebEngine Quick QuickControls2 X11Extras DBus Xml Concurrent LinguistTools)
set(QT Core Gui Widgets Network Quick QuickControls2 X11Extras DBus Xml Concurrent LinguistTools)
find_package(Qt5 REQUIRED ${QT})
find_package(X11 REQUIRED)
find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(Libcrypt REQUIRED)

find_package(KF5NetworkManagerQt REQUIRED)
find_package(KF5ModemManagerQt REQUIRED)
find_package(KF5Config)

find_package(LingmoLogger REQUIRED)

pkg_search_module(FontConfig REQUIRED fontconfig IMPORTED_TARGET)
pkg_search_module(ICU REQUIRED icu-i18n)

include_directories(${ICU_INCLUDE_DIRS})

file(GLOB_RECURSE SRCS_R "include/*.h" "src/*.c" "src/*.cpp" "src/*.h")
foreach(filepath ${SRCS_R})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND SRCS ${filename})
endforeach()

set(RESOURCES
    src/resources.qrc
)

qt5_add_dbus_adaptor(DBUS_SOURCES
                     src/com.lingmo.SettingsUI.xml
                     src/application.h Application)
set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

# qt5_add_dbus_interface(DBUS_SOURCES src/datetime/org.freedesktop.timedate1.xml timedated_interface)

add_executable(${PROJECT_NAME} ${DBUS_SOURCES} ${SRCS} ${RESOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC "include" "src")
target_link_libraries(${PROJECT_NAME}
        Qt5::Core
        Qt5::Widgets
        Qt5::Quick
        Qt5::QuickControls2
        Qt5::DBus
        Qt5::Xml
        Qt5::X11Extras
        Qt5::Concurrent

        Qt5::Network

        KF5::NetworkManagerQt
        KF5::ModemManagerQt
        KF5::ConfigCore

        PkgConfig::FontConfig
        ${FREETYPE_LIBRARIES}
        ${ICU_LDFLAGS}
        Libcrypt::Libcrypt

        ${X11_LIBRARIES}
        X11::X11
        X11::Xi
        X11::Xcursor

        LingmoLogger
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

foreach(_ts_file ${TS_FILES})
    execute_process(
        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file})
    execute_process(COMMAND lrelease ${_ts_file})
endforeach()

include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES
    lingmo-settings.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications/
    COMPONENT Runtime
)

install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)

# 安装工具脚本
install(DIRECTORY "tools" DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME})

# dev files
file(GLOB EXPORT_HEADERS LIST_DIRECTORIES true "include/*")
set(INCLUDE_INSTALL_DIR ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})
install(DIRECTORY ${EXPORT_HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})

install(FILES "tools/auto-install-policy/org.lingmo.settings.update.lminstall.policy" DESTINATION "${CMAKE_INSTALL_DATADIR}/polkit-1/actions/")
