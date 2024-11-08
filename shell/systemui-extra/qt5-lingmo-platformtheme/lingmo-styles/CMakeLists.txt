cmake_minimum_required(VERSION 3.16)

project(qt5-style-lingmo)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_CURRENT_SOURCE_DIR}/cmake/kde-modules ${CMAKE_MODULE_PATH})

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()


find_package(Qt5Gui)
message(${imported_location})
set(LIB_PATH ${imported_location})
get_filename_component(PARENT_PATH "${LIB_PATH}" PATH)
get_filename_component(PARENT_PATH "${PARENT_PATH}" PATH)
message("qt5-style-lingmo PARENT_PATH: ${PARENT_PATH}")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    DBus
    Widgets
    X11Extras
)
include_directories(${Qt5Widgets_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})

find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED COMPONENTS)

find_package(KF5Wayland CONFIG REQUIRED COMPONENTS)

find_package(PkgConfig)
pkg_check_modules(Qsettings REQUIRED gsettings-qt)
pkg_check_modules(LINGMOSDKCONF2 REQUIRED lingmosdk-conf2)

include_directories(${Qsettings_INCLUDE_DIRS})
#file(GLOB_RECURSE native_srcs /qt5-config-style-lingmo/*.cpp)
if (LINGMOSDKCONF2_FOUND)
    include_directories(${LINGMOSDKCONF2_INCLUDE_DIRS})
    link_directories(${LINGMOSDKCONF2_LIBRARY_DIRS})
endif()

file(GLOB_RECURSE Header "*.h" "*.hpp" )
file(GLOB_RECURSE Src "*.cpp" "*.c")
SOURCE_GROUP("header" FILES ${Header})
SOURCE_GROUP("src" FILES ${Src})

file(GLOB_RECURSE Json "*.json")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${Header} ${Src} ${Json})


include_directories(../libqt5-lingmo-style)
  add_library(${PROJECT_NAME} MODULE ${Header} ${Src}  ${Json})

  target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/libqt5-lingmo-style/animations)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::DBus
    Qt5::Gui
    Qt5::X11Extras
    KF5::WaylandClient
    KF5::WindowSystem
    xcb
    ${XCB_LIBS}
    gsettings-qt
    qt5-lingmo-style
    ${LINGMOSDKCONF2_LIBRARIES}
    )


#add_subdirectory(qt5-config-style-lingmo)

add_definitions(-DQT5LINGMOPLATFORMTHEME_LIBRARY)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_MESSAGELOGCONTEXT)

if(UNIX)
    set(TARGET_PATH "${PARENT_PATH}/styles")
    MESSAGE("liblingmo-styles TARGET_PATH: ${TARGET_PATH}")

    install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PATH})

    set(JSON_PATH "/usr/share/qt5-lingmo-platformtheme/themeconfig/")
    file(GLOB_RECURSE JSON_FILES0 "qt5-config-style-lingmo/themeconfig/*.json")
    install(FILES ${JSON_FILES0} DESTINATION ${JSON_PATH})
    install(FILES ${JSON_FILES1} DESTINATION ${JSON_PATH})
endif()
