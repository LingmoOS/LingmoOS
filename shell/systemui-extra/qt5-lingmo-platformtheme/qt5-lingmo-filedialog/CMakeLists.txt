cmake_minimum_required(VERSION 3.16)

project(qt5-lingmo-filedialog)

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
message("qt5-lingmo-filedialog PARENT_PATH: ${PARENT_PATH}")
set(TARGET_PATH ${PARENT_PATH})

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    DBus
    Widgets
    X11Extras
)

include_directories(${Qt5Widgets_PRIVATE_INCLUDE_DIRS})
include_directories(${Qt5Gui_PRIVATE_INCLUDE_DIRS})


find_package(Qt5LinguistTools)

find_package(KF5WindowSystem ${KF5_MIN_VERSION} CONFIG REQUIRED COMPONENTS)
find_package(KF5Wayland CONFIG REQUIRED COMPONENTS)

find_package(X11)

find_package(PkgConfig)
pkg_check_modules(QGSETTINGS REQUIRED gsettings-qt)
pkg_check_modules(PEONY REQUIRED explor)
pkg_check_modules(REQUIRED lingmosdk-waylandhelper)
pkg_check_modules(GLIB2 REQUIRED glib-2.0 gio-2.0 )
pkg_check_modules(LINGMOSDKWAYLANDHELPER REQUIRED lingmosdk-waylandhelper)
pkg_check_modules(LINGMOSDKCONF2 REQUIRED lingmosdk-conf2)

include_directories(${QGSETTINGS_INCLUDE_DIRS})
include_directories(${GLIB2_INCLUDE_DIRS})

if (PEONY_FOUND)
    include_directories(${PEONY_INCLUDE_DIRS})
    link_directories(${PEONY_LIBRARY_DIRS})
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

include_directories(../libqt5-lingmo-style/)

file(GLOB_RECURSE TS_FILES
    translations/qt5-lingmo-filedialog_zh_HK.ts
    translations/qt5-lingmo-filedialog_zh_Hant.ts
    translations/qt5-lingmo-filedialog_zh_Hans.ts
    translations/qt5-lingmo-filedialog_zh_CN.ts
    translations/qt5-lingmo-filedialog_ug.ts
    translations/qt5-lingmo-filedialog_tr.ts
    translations/qt5-lingmo-filedialog_mn.ts
    translations/qt5-lingmo-filedialog_ky.ts
    translations/qt5-lingmo-filedialog_kk.ts
    translations/qt5-lingmo-filedialog_fr.ts
    translations/qt5-lingmo-filedialog_fa.ts
    translations/qt5-lingmo-filedialog_es.ts
    translations/qt5-lingmo-filedialog_en_US.ts
    translations/qt5-lingmo-filedialog_de.ts
    translations/qt5-lingmo-filedialog_cs.ts
    translations/qt5-lingmo-filedialog_bo_CN.ts
    )

set(Sources
    sidebar.cpp
    kyfiledialogprivate.cpp
    kyfiledialog.cpp
    uikyfiledialog.cpp
    pathbar.cpp
    menutoolbutoon.cpp
    filedialogplugin.cpp
    sidebar.h
    kyfiledialogprivate.h
    uikyfiledialog.h
    pathbar.h
    menutoolbutoon.h
    kyfiledialog.h
    filedialogplugin.h
    filedialogplugin_global.h
    filedialoginterface.h
    debug.h
    )

set(Json
    filedialogplugin.json)

# i18n
set(LINGMO_TRANSLATIONS_DIR "/usr/share/qt5-lingmo-platformtheme")
add_definitions(
    -DLINGMO_TRANSLATIONS_DIR="${LINGMO_TRANSLATIONS_DIR}"
)
if (NOT DEFINED UPDATE_TRANSLATIONS)
    set(UPDATE_TRANSLATIONS "No")
endif()

# To create a new ts file: lupdate -recursive . -target-language en_US -ts translations/en_US.ts -no-ui-lines
#file(GLOB TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/translations/*.ts")
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TS_FILES})


# cmake -DUPDATE_TRANSLATIONS=yes
if (UPDATE_TRANSLATIONS)
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
endif()

MESSAGE("QM_FILES PATH: ${QM_FILES}")

add_custom_target(translations ALL DEPENDS ${QM_FILES})

add_library(qt5-lingmo-filedialog MODULE ${Sources} ${Json} ${TS_FILES})

target_link_libraries(qt5-lingmo-filedialog
    Qt5::Widgets
    Qt5::DBus
    Qt5::Gui
    Qt5::X11Extras
    KF5::WaylandClient
    KF5::WindowSystem
    ${QGSETTINGS_LIBRARIES}
    ${LINGMOSDKWAYLANDHELPER_LIBRARIES}
    xcb
    glib-2.0
    ${PEONY_LIBRARIES}
    ${XCB_LIBS}
    qt5-lingmo-style
    ${LINGMOSDKCONF2_LIBRARIES}
    )

add_definitions(-DQT5LINGMOPLATFORMTHEME_LIBRARY)
add_definitions(-DQT_DEPRECATED_WARNINGS)
add_definitions(-DQT_MESSAGELOGCONTEXT)

if(UNIX)
    MESSAGE("libqt5-lingmo-filedialog TARGET_PATH: ${TARGET_PATH}")

    install(TARGETS ${PROJECT_NAME} DESTINATION ${TARGET_PATH}/platformthemes/)

    install(FILES ${QM_FILES} DESTINATION "${LINGMO_TRANSLATIONS_DIR}")
endif()

set(test_libs
    Qt5::Widgets
    glib-2.0
    explor
    )

SET(_code "
    #include <glib-object.h>
    #include <explor-qt/search-vfs-uri-parser.h>

    int main()
    {
       Peony::SearchVFSUriParser::addSearchKey(\"\", true);
       return 0;
    }")
SET(_file ${CMAKE_CURRENT_BINARY_DIR}/DetermineHavePeonyInterface.cpp)
FILE(WRITE "${_file}" "${_code}")
TRY_COMPILE(HAVE_PEONY_INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${_file}"
    LINK_LIBRARIES ${test_libs}
    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${GLIB2_INCLUDE_DIRS} ${PEONY_INCLUDE_DIRS}"
    OUTPUT_VARIABLE OUTPUT)
MESSAGE("Build output: ${OUTPUT} HAVE_PEONY_INTERFACE: ${HAVE_PEONY_INTERFACE}")

if(HAVE_PEONY_INTERFACE)
    add_definitions(-DHAVE_PEONY_INTERFACE=1)
endif()


#set(OPEN_ACTIONS_SEPARATOR "open-actions-seperator")
#set(CREATE_ACTIONS_SEPARATOR "create-actions-seperator")
#set(VIEW_ACTIONS_SEPARATOR "view-actions-seperator")
#set(FILE_OPERATION_ACTIONS_SEPARATOR "file-operation-actions-seperator")
#set(PLUGIN_ACTIONS_SEPARATOR "plugin-actions-seperator")
#set(PROPERTIES_ACTIONS_SEPARATOR "properties-actions-seperator")
#set(COMPUTER_ACTIONS_SEPARATOR "computer-actions-seperator")
#set(TRASH_ACTIONS_SEPARATOR "trash-actions-seperator")
#set(OPEN_IN_NEW_WINDOW_ACTION "open-in-new-window-action")
#set(OPEN_IN_NEW_TAB_ACTION "open-in-new-tab-action")
#set(ADD_TO_BOOKMARK_ACTION "add-to-bookmark-action")
#set(OPEN_ACTION "open-action")
#set(OPEN_WITH_ACTION "open-with-action")
#set(OPEN_SELECTED_FILES_ACTION "open-selected-files-action")
#set(CREATE_ACTION "create-action")
#set(VIEW_TYPE_ACTION "view-type-action")
#set(SORT_TYPE_ACTION "sort-type-action")
#set(SORT_ORDER_ACTION "sort-order-action")
#set(SORT_PREFERENCES_ACTION "sort-preferences-action")
#set(COPY_ACTION "copy-action")
#set(CUT_ACTION "cut-action")
#set(TRASH_ACTION "trash-action")
#set(DELETE_ACTION "delete-action")
#set(RENAME_ACTION "rename-action")
#set(PASTE_ACTION "paste-action")
#set(REFRESH_ACTION "refresh-action")
#set(SELECT_ALL_ACTION "select-all-action")
#set(REVERSE_SELECT_ACTION "reverse-select-action")
#set(PROPERTIES_ACTION "properties-action")
#set(FORMAT_ACTION "format-action")
#set(CLEAN_THE_TRASH_ACTION "clean-the-trash-action")
#set(RESTORE_ACTION "restore-action")
#set(CLEAN_THE_RECENT_ACTION "clean-the-recent-action")

#add_compile_definitions(OPEN_ACTIONS_SEPARATOR=${OPEN_ACTIONS_SEPARATOR})
#add_compile_definitions(CREATE_ACTIONS_SEPARATOR=${CREATE_ACTIONS_SEPARATOR})
#add_compile_definitions(VIEW_ACTIONS_SEPARATOR=${VIEW_ACTIONS_SEPARATOR})
#add_compile_definitions(FILE_OPERATION_ACTIONS_SEPARATOR=${FILE_OPERATION_ACTIONS_SEPARATOR})
#add_compile_definitions(PLUGIN_ACTIONS_SEPARATOR=${PLUGIN_ACTIONS_SEPARATOR})
#add_compile_definitions(PROPERTIES_ACTIONS_SEPARATOR=${PROPERTIES_ACTIONS_SEPARATOR})
#add_compile_definitions(COMPUTER_ACTIONS_SEPARATOR=${COMPUTER_ACTIONS_SEPARATOR})
#add_compile_definitions(TRASH_ACTIONS_SEPARATOR=${TRASH_ACTIONS_SEPARATOR})
#add_compile_definitions(OPEN_IN_NEW_WINDOW_ACTION=${OPEN_IN_NEW_WINDOW_ACTION})
#add_compile_definitions(OPEN_IN_NEW_TAB_ACTION=${OPEN_IN_NEW_TAB_ACTION})
#add_compile_definitions(ADD_TO_BOOKMARK_ACTION=${ADD_TO_BOOKMARK_ACTION})
#add_compile_definitions(OPEN_ACTION=${OPEN_ACTION})
#add_compile_definitions(OPEN_WITH_ACTION=${OPEN_WITH_ACTION})
#add_compile_definitions(OPEN_SELECTED_FILES_ACTION=${OPEN_SELECTED_FILES_ACTION})
#add_compile_definitions(CREATE_ACTION=${CREATE_ACTION})
#add_compile_definitions(VIEW_TYPE_ACTION=${VIEW_TYPE_ACTION})
#add_compile_definitions(SORT_TYPE_ACTION=${SORT_TYPE_ACTION})
#add_compile_definitions(SORT_ORDER_ACTION=${SORT_ORDER_ACTION})
#add_compile_definitions(SORT_PREFERENCES_ACTION=${SORT_PREFERENCES_ACTION})
#add_compile_definitions(COPY_ACTION=${COPY_ACTION})
#add_compile_definitions(CUT_ACTION=${CUT_ACTION})
#add_compile_definitions(TRASH_ACTION=${TRASH_ACTION})
#add_compile_definitions(DELETE_ACTION=${DELETE_ACTION})
#add_compile_definitions(RENAME_ACTION=${RENAME_ACTION})
#add_compile_definitions(PASTE_ACTION=${PASTE_ACTION})
#add_compile_definitions(REFRESH_ACTION=${REFRESH_ACTION})
#add_compile_definitions(SELECT_ALL_ACTION=${SELECT_ALL_ACTION})
#add_compile_definitions(REVERSE_SELECT_ACTION=${REVERSE_SELECT_ACTION})
#add_compile_definitions(PROPERTIES_ACTION=${PROPERTIES_ACTION})
#add_compile_definitions(FORMAT_ACTION=${FORMAT_ACTION})
#add_compile_definitions(CLEAN_THE_TRASH_ACTION=${CLEAN_THE_TRASH_ACTION})
#add_compile_definitions(RESTORE_ACTION=${RESTORE_ACTION})
#add_compile_definitions(CLEAN_THE_RECENT_ACTION=${CLEAN_THE_RECENT_ACTION})

