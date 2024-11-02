cmake_minimum_required(VERSION 3.14)
project(lingmo-quick-items-plugin)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find QT modules
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Qml Quick Widgets Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Qml Quick Widgets Gui REQUIRED)
find_package(KF5WindowSystem REQUIRED)
find_package(PkgConfig REQUIRED)

set(EXTERNAL_LIBS "")
set(PC_PKGS Qt5Xdg)

foreach(PC_LIB IN ITEMS ${PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(LINGMO_QUICK_CORE_ITEMS_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(PLUGIN_INSTALL_ROOT_PATH "/usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/qt5/qml/org/lingmo/quick/items")

include_directories(views)
# items模块的功能不需要导出到include目录，仅使用qml的import语句进行导入使用
set (ITEMS_PLUGIN_SRCS
        lingmo-quick-items-plugin.cpp lingmo-quick-items-plugin.h
        menu.cpp menu.h
        menu-item.cpp menu-item.h
        theme-icon.cpp theme-icon.h
        tooltip-proxy.cpp tooltip-proxy.h
        icon-provider.cpp icon-provider.h
        icon.cpp icon.h
        icon-helper.cpp icon-helper.h
        color-mixer.cpp color-mixer.h
        action-extension.cpp action-extension.h
        views/content-window.cpp views/content-window.h
        views/tooltip.cpp views/tooltip.h
        views/lingmo-style-window.cpp views/lingmo-style-window.h
        views/tooltip-dialog.cpp views/tooltip-dialog.h
        window-blur-behind.cpp window-blur-behind.h
)

set(QRC_FILES qml/qml.qrc)

add_library(${PROJECT_NAME} SHARED ${ITEMS_PLUGIN_SRCS} ${QRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ../core)
target_include_directories(${PROJECT_NAME} PRIVATE ../platform)
target_include_directories(${PROJECT_NAME} PRIVATE ../framework)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLUGIN_IMPORT_URI="org.lingmo.quick.items"
        PLUGIN_VERSION_MAJOR=${VERSION_MAJOR}
        PLUGIN_VERSION_MINOR=${VERSION_MINOR}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Qml
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Gui
        Qt::GuiPrivate
        KF5::WindowSystem
        lingmo-quick::core
        lingmo-quick::platform
        lingmo-quick::framework
        ${EXTERNAL_LIBS}
)

install(DIRECTORY "qml" DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")
install(FILES qmldir DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")
install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION "${PLUGIN_INSTALL_ROOT_PATH}")
