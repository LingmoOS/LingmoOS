cmake_minimum_required(VERSION 3.16)

project(qt5-lingmo-platformtheme)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT_MIN_VERSION "5.12.0")
set(KF5_MIN_VERSION "5.66.0")

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
message("qt5-lingmo-platformtheme PARENT_PATH: ${PARENT_PATH}")
set(TARGET_PATH ${PARENT_PATH})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    DBus
    Widgets
    X11Extras
    Quick
    QuickControls2
)

foreach(_dir ${Qt5Gui_PRIVATE_INCLUDE_DIRS})
    message(${_dir})

    string(REGEX MATCHALL "QtGui" matches "${_dir}")
    list(LENGTH matches match_count)
#    message(STATUS "字符串中包含 ${match_count} 个 'QtGui'")
    if(${match_count} EQUAL 2)
        string(REPLACE "QtGui" "QtThemeSupport" ThemeSupport_string "${_dir}")
    endif()
endforeach()
message(${ThemeSupport_string})
include_directories(${ThemeSupport_string})

include_directories(${Qt5Widgets_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})


find_package(Qt5LinguistTools)

find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED COMPONENTS)
find_package(KF5Wayland CONFIG REQUIRED COMPONENTS)

find_package(X11)

find_package(PkgConfig)
pkg_check_modules(QGSETTINGS REQUIRED gsettings-qt)
pkg_check_modules(REQUIRED Qt5XdgIconLoader)
pkg_check_modules(REQUIRED lingmosdk-waylandhelper)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)
pkg_check_modules(FREETYPE2 REQUIRED freetype2)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 )
pkg_check_modules(LINGMOSDKWAYLANDHELPER REQUIRED lingmosdk-waylandhelper)
pkg_check_modules(LINGMOSDKCONF2 REQUIRED lingmosdk-conf2)

include_directories(${QGSETTINGS_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIRS})

if (FONTCONFIG_FOUND)
    include_directories(${FONTCONFIG_INCLUDE_DIRS})
    link_directories(${FONTCONFIG_LIBRARY_DIRS})
endif()
if (FREETYPE2_FOUND)
    include_directories(${FREETYPE2_INCLUDE_DIRS})
    link_directories(${FREETYPE2_LIBRARY_DIRS})
endif()
if (LINGMOSDKWAYLANDHELPER_FOUND)
    include_directories(${LINGMOSDKWAYLANDHELPER_INCLUDE_DIRS})
    link_directories(${LINGMOSDKWAYLANDHELPER_LIBRARY_DIRS})
endif()
if (QGSETTINGS_FOUND)
    include_directories(${QGSETTINGS_INCLUDE_DIRS})
    link_directories(${QGSETTINGS_LIBRARY_DIRS})
endif()
if (LINGMOSDKCONF2_FOUND)
    include_directories(${LINGMOSDKCONF2_INCLUDE_DIRS})
    link_directories(${LINGMOSDKCONF2_LIBRARY_DIRS})
endif()


#message(${GLIB2_INCLUDE_DIRS})
#message(${FREETYPE2_INCLUDE_DIRS})

include_directories(../libqt5-lingmo-style/)

file(GLOB_RECURSE sources "*.h" "*.hpp" "*.cpp" "*.c" "*.ui")
file(GLOB_RECURSE Json "*.json")

message("jsonfiles path:${Json}")

# To create a new ts file: lupdate -recursive . -target-language en_US -ts translations/en_US.ts -no-ui-lines
file(GLOB TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${sources} ${Json} ${TS_FILES})

# i18n
set(LINGMO_TRANSLATIONS_DIR "/usr/share/qt5-lingmo-platformtheme")
add_definitions(
    -DLINGMO_TRANSLATIONS_DIR="${LINGMO_TRANSLATIONS_DIR}"
)
if (NOT DEFINED UPDATE_TRANSLATIONS)
    set(UPDATE_TRANSLATIONS "No")
endif()

# cmake -DUPDATE_TRANSLATIONS=yes
if (UPDATE_TRANSLATIONS)
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

MESSAGE("QM_FILES PATH: ${QM_FILES}")

add_custom_target(themetranslations ALL DEPENDS ${QM_FILES})

add_library(qt5-lingmo-platformtheme MODULE ${sources} ${Json} ${TS_FILES})

get_filename_component(PARENT_PATH "${PARENT_PATH}" PATH)
get_filename_component(PARENT_PATH "${PARENT_PATH}" PATH)
set(themesupport_dir ${PARENT_PATH})

message("_qt5Widgets_install_prefix_path:${themesupport_dir}")
target_link_libraries(qt5-lingmo-platformtheme
    Qt5::Widgets
    Qt5::DBus
    Qt5::Gui
    Qt5::X11Extras
    Qt5::Quick
    Qt5::QuickControls2
    KF5::WaylandClient
    KF5::WindowSystem
    ${QGSETTINGS_LIBRARIES}
    ${LINGMOSDKWAYLANDHELPER_LIBRARIES}
    xcb
    glib-2.0
    ${XCB_LIBS}
    qt5-lingmo-style
    ${LINGMOSDKCONF2_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${FREETYPE2_LIBRARIES}
    "${themesupport_dir}/libQt5ThemeSupport.a" #link Qt5ThemeSupport
    )

add_definitions(-DQT5LINGMOPLATFORMTHEME_LIBRARY)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_MESSAGELOGCONTEXT)

set(LINGMOPLATFORMTHEME_DIR ${TARGET_PATH})
target_compile_definitions(${PROJECT_NAME} PRIVATE LINGMOPLATFORMTHEME_DIR="${LINGMOPLATFORMTHEME_DIR}")

message("DLINGMOPLATFORMTHEME_DIR path: ${LINGMOPLATFORMTHEME_DIR}")
if(UNIX)
    MESSAGE("libqt5-lingmo-platformtheme TARGET_PATH: ${TARGET_PATH}")

    install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PATH}/platformthemes/)

    install(FILES ${QM_FILES} DESTINATION "${LINGMO_TRANSLATIONS_DIR}")
endif()
