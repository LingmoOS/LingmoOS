cmake_minimum_required(VERSION 3.5)

project(liblingmo)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)

include(GenerateExportHeader)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(QT Core Gui Quick QuickControls2 Widgets DBus Xml Concurrent)
find_package(Qt5 REQUIRED ${QT})

# Get the installation directory from qmake
get_target_property(QT_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)
if(NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "qmake is not found.")
endif()

execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_QML
    OUTPUT_VARIABLE INSTALL_QMLDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(INSTALL_QMLDIR)
    message(STATUS "qml directory:" "${INSTALL_QMLDIR}")
else()
    message(FATAL_ERROR "qml directory cannot be detected.")
endif()
# -------------------------------------------
add_subdirectory(accounts)
add_subdirectory(bluez)
add_subdirectory(mpris)
add_subdirectory(networkmanagement)
add_subdirectory(screen)
add_subdirectory(system)
add_subdirectory(audio)
add_subdirectory(easylogger)
