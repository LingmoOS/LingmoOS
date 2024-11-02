cmake_minimum_required(VERSION 3.14)
project(lingmo-search-service VERSION 1.0.0 LANGUAGES C CXX)

set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_MICRO 0)
set(LINGMO_SEARCH_SERVICE_VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_MICRO})
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core DBus Gui Quick Widgets RemoteObjects REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core DBus Gui Quick Widgets RemoteObjects REQUIRED)
find_package(PkgConfig REQUIRED)
set(LINGMO_SEARCH_SERVICE_EXTERNAL_LIBS "")
set(LINGMO_SEARCH_SERVICE_PC_PKGS gsettings-qt)

foreach(PC_LIB IN ITEMS ${LINGMO_SEARCH_SERVICE_PC_PKGS})
    pkg_check_modules(${PC_LIB} REQUIRED IMPORTED_TARGET ${PC_LIB})
    if(${${PC_LIB}_FOUND})
        include_directories(${${PC_LIB}_INCLUDE_DIRS})
        link_directories(${${PC_LIB}_LIBRARY_DIRS})
        list(APPEND LINGMO_SEARCH_SERVICE_EXTERNAL_LIBS PkgConfig::${PC_LIB})
    endif()
endforeach()

set(QRC_FILES qml/qml.qrc)
set(LINGMO_SEARCH_SERVICE_SRC
        main.cpp
        lingmo-search-service.cpp
        lingmo-search-service.h
        monitor.cpp
        monitor.h
        cgroup-helper.cpp
        cgroup-helper.h
)
qt5_generate_repc(LINGMO_SEARCH_SERVICE_SRC ../libsearch/index/monitor.rep SOURCE)
add_executable(lingmo-search-service
        ${LINGMO_SEARCH_SERVICE_SRC}
        ${QRC_FILES}
)
target_include_directories(lingmo-search-service PRIVATE
    ../3rd-parties/qtsingleapplication/src
    ../libsearch
    ../libsearch/dirwatcher
    ../libsearch/filesystemwatcher
    ../libsearch/index
    ../libsearch/parser
)

target_compile_definitions(lingmo-search-service PRIVATE
        QT_DEPRECATED_WARNINGS
        VERSION="${LINGMO_SEARCH_SERVICE_VERSION}"
        QT_NO_KEYWORDS
        )

target_link_libraries(lingmo-search-service PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::DBus
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::Quick
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::RemoteObjects
        lingmo-search
        qtsingleapplication
        ${LINGMO_SEARCH_SERVICE_EXTERNAL_LIBS}
)
install(FILES ../data/org.lingmo.search.data.gschema.xml DESTINATION /usr/share/glib-2.0/schemas/)
install(FILES ../data/lingmo-search-service.desktop DESTINATION /etc/xdg/autostart)
install(TARGETS lingmo-search-service DESTINATION /usr/bin)
