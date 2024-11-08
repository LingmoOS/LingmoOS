cmake_minimum_required(VERSION 3.16)
project(recent-file)

set(EXTENSION_NAME "recent-file")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5
        COMPONENTS Core Gui Quick Widgets DBus LinguistTools REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}
        COMPONENTS Core Gui Quick Widgets DBus LinguistTools REQUIRED)

find_package(lingmo-menu REQUIRED)

set(SOURCE recent-file-extension.cpp recent-file-extension.h
        event-track.cpp event-track.h
)

find_package(PkgConfig REQUIRED)
set(EXTERNAL_LIBS "")
set(PC_PKGS glib-2.0 gio-unix-2.0 lingmosdk-datacollect lingmosdk-systime)

foreach(lib IN ITEMS ${PC_PKGS})
    pkg_check_modules(${lib} REQUIRED ${lib})
    if(${${lib}_FOUND})
        include_directories(${${lib}_INCLUDE_DIRS})
        link_directories(${${lib}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS ${${lib}_LIBRARIES})
    endif()
endforeach()

set(QRC_FILES qml/qml.qrc)
# 翻译文件
file(GLOB TS_FILES "translations/*.ts")
# 更新翻译并创建.qm文件
qt5_create_translation(QM_FILES ${PROJECT_SOURCE_DIR} ${TS_FILES})

set(RECENT_FILE_TRANSLATION_DIR "/usr/share/lingmo-menu/extensions/translations/")
add_compile_definitions(RECENT_FILE_TRANSLATION_DIR="${RECENT_FILE_TRANSLATION_DIR}")

add_library(${EXTENSION_NAME} SHARED ${SOURCE} ${QM_FILES} ${QRC_FILES})
target_link_libraries(${EXTENSION_NAME} PRIVATE
        Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Quick Qt5::DBus
        ${EXTERNAL_LIBS}
        lingmo-menu
)

install(TARGETS ${EXTENSION_NAME} LIBRARY DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/lingmo-menu/extensions")
install(FILES ${QM_FILES} DESTINATION ${RECENT_FILE_TRANSLATION_DIR})
