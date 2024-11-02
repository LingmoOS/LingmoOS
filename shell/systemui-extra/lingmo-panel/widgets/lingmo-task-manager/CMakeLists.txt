cmake_minimum_required(VERSION 3.14)

#project(lingmo-task-manager LANGUAGES CXX)
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(LINGMO_TASK_MANAGER_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core LinguistTools DBus Widgets Quick X11Extras Qml REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core LinguistTools DBus Widgets Quick X11Extras Qml REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(KF5WindowSystem)
find_package(KF5Wayland)
find_package(Qt5WaylandClient)
find_package(lingmo-quick COMPONENTS core framework platform)
find_package(Qt5Xdg REQUIRED)

set(LINGMO_TASK_MANAGER_PC_PKGS lingmosdk-waylandhelper lingmo-search)
foreach(external_lib IN ITEMS ${LINGMO_TASK_MANAGER_PC_PKGS})
    pkg_check_modules(${external_lib} REQUIRED IMPORTED_TARGET ${external_lib})
    if(${${external_lib}_FOUND})
        include_directories(${${external_lib}_INCLUDE_DIRS})
        link_directories(${${external_lib}_LIBRARY_DIRS})
        list(APPEND LINGMO_TASK_MANAGER_EXTERNAL_LIBS PkgConfig::${external_lib})
    endif()
endforeach()

file(GLOB TS_FILES *.ts)

set(TRANSLATION_FILE_DIR "/usr/share/lingmo-task-manager/translations")
add_compile_definitions(TRANSLATION_FILE_DIR="${TRANSLATION_FILE_DIR}"
        LINGMO_TASK_MANAGER_VERSION="${LINGMO_TASK_MANAGER_VERSION}")

set(lingmoTaskManager_SRC
        lingmo-task-manager.cpp lingmo-task-manager.h
        task-manager-item.cpp task-manager-item.h
        utils.cpp utils.h
        actions.cpp actions.h
        task-manager-plugin.cpp task-manager-plugin.h
        window-thumbnail-view.cpp window-thumbnail-view.h
        task-manager-filter-model.cpp
        task-manager-filter-model.h
)
if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(lingmoTaskManager_SRC dbus/org.lingmo.TaskManager.xml lingmo-task-manager.h TaskManager::LingmoUITaskManager)
else()
    qt5_add_dbus_adaptor(lingmoTaskManager_SRC dbus/org.lingmo.TaskManager.xml lingmo-task-manager.h TaskManager::LingmoUITaskManager)
endif()

if(COMMAND qt_create_translation)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${TS_FILES} OPTIONS -no-obsolete -no-ui-lines)
endif()

set(QRC_FILES qml/qml.qrc)

add_library(lingmo-task-manager SHARED
        ${lingmoTaskManager_SRC}
        ${QRC_FILES}
        ${QM_FILES}
        )
target_include_directories(lingmo-task-manager PUBLIC
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/framework/config>
)

target_link_libraries(lingmo-task-manager PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::X11Extras
        Qt${QT_VERSION_MAJOR}::Qml
        ${LINGMO_TASK_MANAGER_EXTERNAL_LIBS}
        KF5::WindowSystem
        KF5::WaylandClient
        Qt5Xdg
        lingmo-quick::core
        lingmo-quick::framework
        lingmo-quick::platform
 )

set_target_properties(lingmo-task-manager PROPERTIES
        VERSION ${LINGMO_TASK_MANAGER_VERSION}
        SOVERSION ${VERSION_MAJOR}
        OUTPUT_NAME lingmo-task-manager
        )

install(FILES ${QM_FILES} DESTINATION "${TRANSLATION_FILE_DIR}")

set(PLUGIN_INSTALL_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/lingmo/panel/taskManager")

install(DIRECTORY "plugin/" DESTINATION "/usr/share/lingmo/widgets/org.lingmo.panel.taskManager")
install(FILES "qmldir" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/AppIcon.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/AppList.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/TaskManager.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/TaskItemData.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/TaskManagerView.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/ThumbnailWindow.qml" DESTINATION ${PLUGIN_INSTALL_PATH})
install(FILES "qml/ThumbnailContainer.qml" DESTINATION ${PLUGIN_INSTALL_PATH})

install(TARGETS "lingmo-task-manager" LIBRARY DESTINATION ${PLUGIN_INSTALL_PATH})

if (BUILD_TEST)
    add_subdirectory(test)
endif ()
