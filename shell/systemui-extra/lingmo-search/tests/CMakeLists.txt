cmake_minimum_required(VERSION 3.16)
project(tests VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)

add_executable(tests
    file-system-watcher-test.cpp file-system-watcher-test.h
    main.cpp
)
target_include_directories(tests PRIVATE
    ../libsearch ../libsearch
    ../libsearch/appsearch
    ../libsearch/dirwatcher
    ../libsearch/filesystemwatcher
    ../libsearch/index
    ../libsearch/parser
    ../libsearch/plugininterface
    ../libsearch/pluginmanage
    ../libsearch/searchinterface
    ../libsearch/settingsearch
)
target_link_libraries(tests PRIVATE
    # Remove: L${CMAKE_CURRENT_BINARY_DIR}/../libsearch/
    Qt${QT_VERSION_MAJOR}::Core
    lingmo-search
)
