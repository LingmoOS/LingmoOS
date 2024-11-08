cmake_minimum_required(VERSION 3.16)

project(lingmo-menu LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# see https://cmake.org/cmake/help/v3.16/manual/cmake-qt.7.html
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# 查找qt组件
find_package(QT NAMES Qt6 Qt5
        COMPONENTS Core Gui Quick Widgets LinguistTools DBus X11Extras REQUIRED)
find_package(Qt${QT_VERSION_MAJOR}
        COMPONENTS Core Gui Quick Widgets LinguistTools DBus X11Extras REQUIRED)

# find kde framework components
# see: https://api.kde.org/frameworks/index.html > KWindowSystem
find_package(KF5WindowSystem REQUIRED)
find_package(Qt5Xdg REQUIRED)
find_package(lingmo-quick COMPONENTS platform REQUIRED)

# 查找其他组件
# see: https://cmake.org/cmake/help/v3.16/module/FindPkgConfig.html
# see: https://cmake.org/cmake/help/v3.16/command/list.html
find_package(PkgConfig REQUIRED)

set(LINGMO_MENU_EXTERNAL_LIBS "")
# glib-2.0 gio-unix-2.0 gsettings-qt x11 lingmosdk-waylandhelper
set(LINGMO_MENU_PC_PKGS gsettings-qt lingmo-search lingmosdk-datacollect)

foreach(external_lib IN ITEMS ${LINGMO_MENU_PC_PKGS})
        pkg_check_modules(${external_lib} REQUIRED ${external_lib})
        if(${${external_lib}_FOUND})
                include_directories(${${external_lib}_INCLUDE_DIRS})
                link_directories(${${external_lib}_LIBRARY_DIRS})
                list(APPEND LINGMO_MENU_EXTERNAL_LIBS ${${external_lib}_LIBRARIES})
        endif()
endforeach()

message(STATUS "External libraries found: ${LINGMO_MENU_EXTERNAL_LIBS}")

# include single-application
add_subdirectory(3rd-parties/qtsingleapplication)
# static lib of single-application.
set(SingleApplication "qtsingleapplication")

# include文件夹
include_directories(src)
#include_directories(src/model)
include_directories(src/appdata)
include_directories(src/libappdata)
include_directories(src/settings)
include_directories(src/uiconfig)
include_directories(src/windows)
include_directories(src/extension)
include_directories(src/utils)
include_directories(3rd-parties/qtsingleapplication/src)

# 用于Qt Creator识别自定义qml模块的导入路径
list(APPEND QML_MODULE_DIRS "${PROJECT_SOURCE_DIR}/qml")
set(QML_IMPORT_PATH "${QML_MODULE_DIRS}" CACHE STRING "Qt Creator extra qml import paths." FORCE)
#message(STATUS "QML_IMPORT_PATH: ${QML_IMPORT_PATH}")

# 基础设置
set(LINGMO_MENU_DATA_DIR "/usr/share/lingmo-menu")
set(LINGMO_MENU_SO_DIR "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/lingmo-menu")
set(LINGMO_MENU_TRANSLATION_DIR "${LINGMO_MENU_DATA_DIR}/translations")
set(LINGMO_MENU_GLOBAL_CONFIG_FILE "${LINGMO_MENU_DATA_DIR}/lingmo-menu-global-config.conf")
set(LINGMO_MENU_EXTENSION_DIR "${LINGMO_MENU_SO_DIR}/extensions")
set(LINGMO_MENU_CONTEXT_MENU_DIR "${LINGMO_MENU_SO_DIR}/context-menu")


set(LINGMO_MENU_LIBRARY_VERSION 1.0.0)
set(LINGMO_MENU_LIBRARY_API_VERSION 1)
#set(LINGMO_MENU_LIBRARY_NAME "lingmo-menu${LINGMO_MENU_LIBRARY_API_VERSION}")
set(LINGMO_MENU_LIBRARY_NAME "lingmo-menu")
set(LINGMO_MENU_LIBRARY_TARGET "liblingmo-menu")
set(PC_INSTALL_DIR "/usr/lib/pkgconfig")
set(CMAKE_CONFIG_INSTALL_DIR "/usr/share/cmake/${LINGMO_MENU_LIBRARY_NAME}")

# 宏定义
add_compile_definitions(LINGMO_MENU_TRANSLATION_DIR="${LINGMO_MENU_TRANSLATION_DIR}"
        LINGMO_MENU_DATA_DIR="${LINGMO_MENU_DATA_DIR}"
        LINGMO_MENU_CONTEXT_MENU_DIR="${LINGMO_MENU_CONTEXT_MENU_DIR}"
        LINGMO_MENU_EXTENSION_DIR="${LINGMO_MENU_EXTENSION_DIR}"
        LINGMO_MENU_GLOBAL_CONFIG_FILE="${LINGMO_MENU_GLOBAL_CONFIG_FILE}"
        )

# lingmo-menu的源码
set(SOURCE_FILES
        src/main.cpp
        src/commons.h src/commons.cpp
        src/menu-dbus-service.cpp src/menu-dbus-service.h
        src/lingmo-menu-application.cpp src/lingmo-menu-application.h
        src/windows/menu-main-window.cpp src/windows/menu-main-window.h
        src/settings/settings.cpp src/settings/settings.h
        src/settings/user-config.cpp src/settings/user-config.h
        src/utils/power-button.cpp src/utils/power-button.h
        src/utils/app-manager.cpp src/utils/app-manager.h
        src/utils/event-track.cpp src/utils/event-track.h
        src/utils/sidebar-button-utils.cpp src/utils/sidebar-button-utils.h
        src/extension/menu-extension-plugin.cpp src/extension/menu-extension-plugin.h
        src/extension/menu-extension-loader.cpp src/extension/menu-extension-loader.h
        src/extension/widget-extension.cpp src/extension/widget-extension.h
        src/extension/context-menu-extension.cpp src/extension/context-menu-extension.h
        src/extension/context-menu-manager.cpp src/extension/context-menu-manager.h
        src/extension/widget-extension-model.cpp src/extension/widget-extension-model.h
        src/extension/widget-model.cpp src/extension/widget-model.h
        src/extension/menu/app-menu-plugin.cpp src/extension/menu/app-menu-plugin.h
        src/extension/favorite/folder-model.cpp src/extension/favorite/folder-model.h
        src/extension/favorite/favorites-model.cpp src/extension/favorite/favorites-model.h
        src/extension/favorite/favorite-widget.cpp src/extension/favorite/favorite-widget.h
        src/extension/favorite/favorites-config.cpp src/extension/favorite/favorites-config.h
        src/extension/favorite/app-favorite-model.cpp src/extension/favorite/app-favorite-model.h
        src/extension/favorite/favorite-context-menu.cpp src/extension/favorite/favorite-context-menu.h
        src/extension/favorite/favorite-folder-helper.cpp src/extension/favorite/favorite-folder-helper.h
        src/extension/favorite/favorite-extension-plugin.cpp src/extension/favorite/favorite-extension-plugin.h
        src/libappdata/basic-app-model.cpp src/libappdata/basic-app-model.h
        src/libappdata/app-database-interface.cpp src/libappdata/app-database-interface.h
        src/libappdata/app-category-model.cpp src/libappdata/app-category-model.h
        src/libappdata/combined-list-model.cpp src/libappdata/combined-list-model.h
        src/libappdata/recently-installed-model.cpp src/libappdata/recently-installed-model.h
        src/libappdata/app-page-backend.cpp src/libappdata/app-page-backend.h
        src/libappdata/app-list-model.cpp src/libappdata/app-list-model.h
        src/libappdata/app-list-plugin.cpp src/libappdata/app-list-plugin.h
        src/libappdata/app-search-plugin.cpp src/libappdata/app-search-plugin.h
        src/libappdata/app-category-plugin.cpp src/libappdata/app-category-plugin.h
        src/libappdata/app-group-model.cpp src/libappdata/app-group-model.h
)


if(COMMAND qt_add_dbus_adaptor)
        qt_add_dbus_adaptor(SOURCE_FILES data/org.lingmo.menu.xml menu-dbus-service.h LingmoUIMenu::MenuDbusService)
else()
        qt5_add_dbus_adaptor(SOURCE_FILES data/org.lingmo.menu.xml menu-dbus-service.h LingmoUIMenu::MenuDbusService)
endif()

# library sources
set(LIBRARY_SOURCES
        src/data-entity.cpp
        src/extension/menu-extension-plugin.cpp
        src/extension/widget-extension.cpp
        src/extension/context-menu-extension.cpp
)

set(LIBRARY_HEADERS_DIR "/usr/include/${LINGMO_MENU_LIBRARY_NAME}")
set(LIBRARY_HEADERS
        src/data-entity.h
        src/extension/menu-extension-plugin.h
        src/extension/widget-extension.h
        src/extension/context-menu-extension.h
        )

# qrc文件
set(QRC_FILES qml/qml.qrc res/res.qrc)
# desktop file
set(DESKTOP_FILE data/lingmo-menu.desktop)
set(GSETTING_FILE data/org.lingmo.menu.settings.gschema.xml)
set(GLOBAL_CONFIG_FILE data/lingmo-menu-global-config.conf)
set(DBUS_SERVICE_FILE data/org.lingmo.menu.service)
# data files
#set(DATA_FILES data/xxx)

# 翻译文件
file(GLOB TS_FILES "${PROJECT_SOURCE_DIR}/translations/*.ts")
# 更新翻译并创建.qm文件
qt5_create_translation(QM_FILES ${PROJECT_SOURCE_DIR} ${TS_FILES})
# see https://cmake.org/cmake/help/v3.16/command/add_custom_command.html
# add_custom_target(GEN_TS ALL DEPENDS ${TS_FILES})
# add_custom_target(generate_qm ALL DEPENDS ${QM_FILES})

add_library(${LINGMO_MENU_LIBRARY_TARGET} SHARED ${LIBRARY_SOURCES})
set_target_properties(${LINGMO_MENU_LIBRARY_TARGET} PROPERTIES
        VERSION ${LINGMO_MENU_LIBRARY_VERSION}
        SOVERSION ${LINGMO_MENU_LIBRARY_API_VERSION}
        OUTPUT_NAME "lingmo-menu"
        )
target_link_libraries(${LINGMO_MENU_LIBRARY_TARGET} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets)

add_executable(
        ${PROJECT_NAME}
        ${QRC_FILES}
        ${QM_FILES}
        ${SOURCE_FILES}
        )

target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:LINGMO_MENU_LOG_FILE_DISABLE>)
target_compile_definitions(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:QT_QML_DEBUG>)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Quick
        Qt5::Widgets
        Qt5::DBus
        Qt5::X11Extras
        KF5::WindowSystem
        Qt5Xdg
        ${SingleApplication}
        ${LINGMO_MENU_EXTERNAL_LIBS}
        ${LINGMO_MENU_LIBRARY_TARGET}
        lingmo-quick::platform
        )

# 安装lingmo-menu
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "/usr/bin")
install(TARGETS ${LINGMO_MENU_LIBRARY_TARGET}
        EXPORT ${LINGMO_MENU_LIBRARY_NAME}
        PUBLIC_HEADER DESTINATION ${LIBRARY_HEADERS_DIR}
        LIBRARY DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}"
        )
# 安装翻译文件
install(FILES ${QM_FILES} DESTINATION "${LINGMO_MENU_TRANSLATION_DIR}")
# 安装desktop文件
install(FILES ${DESKTOP_FILE} DESTINATION "/etc/xdg/autostart")
install(FILES ${GSETTING_FILE} DESTINATION "/usr/share/glib-2.0/schemas")
install(FILES ${GLOBAL_CONFIG_FILE} DESTINATION "${LINGMO_MENU_DATA_DIR}")
install(FILES ${DBUS_SERVICE_FILE} DESTINATION "/usr/share/dbus-1/services/")
install(FILES ${LIBRARY_HEADERS} DESTINATION "${LIBRARY_HEADERS_DIR}")
install(DIRECTORY "qml/org" DESTINATION "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml")

## 生成开发配置文件
include(CMakePackageConfigHelpers)
target_include_directories(${LINGMO_MENU_LIBRARY_NAME} PUBLIC  $<INSTALL_INTERFACE:${LIBRARY_HEADERS_DIR}>)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${LINGMO_MENU_LIBRARY_NAME}-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${LINGMO_MENU_LIBRARY_NAME}-config.cmake"
        INSTALL_DESTINATION  ${CMAKE_CONFIG_INSTALL_DIR}
)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${LINGMO_MENU_LIBRARY_NAME}-config-version.cmake
        VERSION ${LINGMO_MENU_LIBRARY_VERSION}
        COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${LINGMO_MENU_LIBRARY_NAME}.pc.in"
        "${CMAKE_CURRENT_BINARY_DIR}/cmake/${LINGMO_MENU_LIBRARY_NAME}.pc"
        INSTALL_DESTINATION  ${PC_INSTALL_DIR}
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cmake/${LINGMO_MENU_LIBRARY_NAME}.pc DESTINATION ${PC_INSTALL_DIR})

install(EXPORT ${LINGMO_MENU_LIBRARY_NAME}
        FILE ${LINGMO_MENU_LIBRARY_NAME}-targets.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})

install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${LINGMO_MENU_LIBRARY_NAME}-config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/cmake/${LINGMO_MENU_LIBRARY_NAME}-config-version.cmake
        DESTINATION ${CMAKE_CONFIG_INSTALL_DIR})
