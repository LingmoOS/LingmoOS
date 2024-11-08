set(CMAKE_AUTOMOC on)
find_package(QT NAMES Qt6 Qt5 COMPONENTS Gui Test DBus Qml Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Gui Test DBus Qml Widgets REQUIRED)
find_package(PkgConfig REQUIRED)

add_executable(settings
    ../lingmo/settings.cpp
    test-settings.cpp)
include_directories(../lingmo)
target_link_libraries(settings PRIVATE Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Gui)
add_test(NAME settings COMMAND settings_test)

add_executable(test-theme
    ../lingmo/lingmo-theme-proxy.cpp
    test-theme.cpp)
include_directories(../lingmo)

pkg_check_modules(gsettings-qt REQUIRED IMPORTED_TARGET gsettings-qt)
if(${gsettings-qt_FOUND})
    include_directories(${gsettings-qt_INCLUDE_DIRS})
    link_directories(${gsettings-qt_LIBRARY_DIRS})
    list(APPEND EXTERNAL_LIBS PkgConfig::gsettings-qt)
endif()

target_link_libraries(test-theme PRIVATE Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::DBus
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Widgets
    gsettings-qt)
add_test(NAME test-theme COMMAND test-theme)
