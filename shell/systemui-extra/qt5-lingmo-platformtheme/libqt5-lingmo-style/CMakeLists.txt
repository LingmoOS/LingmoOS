cmake_minimum_required(VERSION 3.16)

project(qt5-lingmo-style)

# 设置共享库的版本号
set(QT5_LINGMO-STYLE_VERSION_MAJOR 1)
set(QT5_LINGMO-STYLE_VERSION_MINOR 0)
set(QT5_LINGMO-STYLE_VERSION_PATCH 0)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

find_package(Qt5Concurrent)
set(LIB_PATH ${imported_location})
get_filename_component(PARENT_PATH "${LIB_PATH}" PATH)
message("libqt5-lingmo-style PARENT_PATH: ${PARENT_PATH}")

find_package(Qt5Widgets)

include_directories(${Qt5Widgets_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

#find_package(Qt5QtThemeSupport REQUIRED)

find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED COMPONENTS)
find_package(KF5Wayland CONFIG REQUIRED COMPONENTS)

find_package(X11)

find_package(PkgConfig)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 )
pkg_check_modules(Qsettings REQUIRED gsettings-qt)
pkg_check_modules(LINGMOSDKCONF2 REQUIRED lingmosdk-conf2)

include_directories(${Qsettings_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIRS})
if (LINGMOSDKCONF2_FOUND)
    include_directories(${LINGMOSDKCONF2_INCLUDE_DIRS})
    link_directories(${LINGMOSDKCONF2_LIBRARY_DIRS})
endif()

file(GLOB_RECURSE Header "*.h" "*.hpp")
file(GLOB_RECURSE Sources "*.cpp" "*.c" "*.ui")
file(GLOB_RECURSE XML "org.lingmo.style.gschema.xml")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Header} ${Sources} ${XML})

add_library(qt5-lingmo-style SHARED ${Sources} ${Header} ${XML})

# 设置共享库的命名规则
set_target_properties(qt5-lingmo-style PROPERTIES
    OUTPUT_NAME "qt5-lingmo-style"
    VERSION ${QT5_LINGMO-STYLE_VERSION_MAJOR}.${QT5_LINGMO-STYLE_VERSION_MINOR}.${QT5_LINGMO-STYLE_VERSION_PATCH}
    SOVERSION ${QT5_LINGMO-STYLE_VERSION_MAJOR}
)

target_link_libraries(qt5-lingmo-style PRIVATE
    Qt5::Widgets
    Qt5::Concurrent
    gsettings-qt
    gio-2.0
    ${LINGMOSDKCONF2_LIBRARIES}
    )


add_definitions(-DLIBQT5LINGMOSTYLE_LIBRARY)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_MESSAGELOGCONTEXT)


if(UNIX)
    set(TARGET_PATH ${PARENT_PATH})
    MESSAGE("libqt5-lingmo-style TARGET_PATH: ${TARGET_PATH}")

    install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PATH})

    set(GSCHEMA_PATH "/usr/share/glib-2.0/schemas")
    set(GSCHEMA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/settings/org.lingmo.style.gschema.xml")
    install(FILES ${GSCHEMA_FILE} DESTINATION ${GSCHEMA_PATH})

    set(KCONF2_PATH "/etc/lingmo-config/basic/")
    set(KCONF2_FILE "${CMAKE_CURRENT_SOURCE_DIR}/settings/org.lingmo.style.yaml")
    install(FILES ${KCONF2_FILE} DESTINATION ${KCONF2_PATH})

    set(PKGCONFIG_PATH "${PARENT_PATH}/pkgconfig")
    set(PKGCONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/development-files/qt5-lingmo.pc")
    install(FILES ${PKGCONFIG_FILE} DESTINATION ${PKGCONFIG_PATH})

    install(FILES ${Header} DESTINATION "libqt5-lingmo-style/")

endif()
