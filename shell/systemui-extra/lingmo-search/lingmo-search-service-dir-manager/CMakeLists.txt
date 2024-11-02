cmake_minimum_required(VERSION 3.14)
project(lingmo-search-service-dir-manager VERSION 1.0.0 LANGUAGES CXX)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(LINGMO_SEARCH_SERVICE_DIR_MANAGER_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Network Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Network Widgets REQUIRED)
find_package(PkgConfig REQUIRED)

set(LINGMO_SEARCH_SERVICE_DIR_MANAGER_EXTERNAL_LIBS "")
set(LINGMO_SEARCH_SERVICE_DIR_MANAGER_PC_PKGS gio-2.0 gio-unix-2.0)

foreach(PC_LIB IN ITEMS ${LINGMO_SEARCH_SERVICE_DIR_MANAGER_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND LINGMO_SEARCH_SERVICE_DIR_MANAGER_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

set(LINGMO_SEARCH_SERVICE_DIR_MANAGER_SRC
        dirwatcher/config.cpp dirwatcher/config.h
        dirwatcher/dir-watcher.cpp dirwatcher/dir-watcher.h
        dirwatcher/search-dir.cpp dirwatcher/search-dir.h
        dirwatcher/volume-manager.cpp dirwatcher/volume-manager.h
        main.cpp
        lingmo-search-dir-manager-dbus.cpp lingmo-search-dir-manager-dbus.h)

if(COMMAND qt_add_dbus_adaptor)
    qt_add_dbus_adaptor(LINGMO_SEARCH_SERVICE_DIR_MANAGER_SRC dirwatcher/com.lingmo.search.FileIndexService.xml dir-watcher.h DirWatcher)
else()
    qt5_add_dbus_adaptor(LINGMO_SEARCH_SERVICE_DIR_MANAGER_SRC dirwatcher/com.lingmo.search.FileIndexService.xml dir-watcher.h DirWatcher)
endif()

add_executable(lingmo-search-service-dir-manager
        ${LINGMO_SEARCH_SERVICE_DIR_MANAGER_SRC}
)
target_include_directories(lingmo-search-service-dir-manager PRIVATE
        ../3rd-parties/qtsingleapplication/src
        dirwatcher
        ../libsearch
        )

target_compile_definitions(lingmo-search-service-dir-manager PRIVATE
    QT_DEPRECATED_WARNINGS
    VERSION="${LINGMO_SEARCH_SERVICE_DIR_MANAGER_VERSION}"
)

target_link_libraries(lingmo-search-service-dir-manager PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Widgets
        qtsingleapplication
        lingmo-search
        ${LINGMO_SEARCH_SERVICE_DIR_MANAGER_EXTERNAL_LIBS}
)
install(FILES conf/com.lingmo.search.fileindex.service DESTINATION /usr/share/dbus-1/services/)
install(FILES ../data/lingmo-search-service-dir-manager.desktop DESTINATION /etc/xdg/autostart)
install(TARGETS lingmo-search-service-dir-manager DESTINATION /usr/bin)