cmake_minimum_required(VERSION 3.14)

project(filemanager LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(QT Core Gui Quick QuickControls2 Widgets DBus Xml Concurrent)
find_package(Qt6 REQUIRED COMPONENTS ${QT} LinguistTools)

# Get the installation directory from qmake
get_target_property(QT_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
if (NOT QT_QMAKE_EXECUTABLE)
    message(FATAL_ERROR "qmake is not found.")
endif ()

execute_process(COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_QML
    OUTPUT_VARIABLE INSTALL_QMLDIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# find_package(LingmoUI REQUIRED)

find_package(KF6 REQUIRED COMPONENTS KIO)
find_package(KF6 REQUIRED COMPONENTS Solid)
find_package(KF6 REQUIRED COMPONENTS WindowSystem)
find_package(KF6 REQUIRED COMPONENTS Config)

qt_add_dbus_adaptor(DBUS_SOURCES
                     com.lingmo.FileManager.xml
                     application.h Application)

add_executable(lingmo-filemanager
  main.cpp
  application.cpp
  window.cpp
  dbusinterface.cpp
  thumbnailer/thumbnailprovider.cpp
  thumbnailer/thumbnailcache.cpp
  runtime/runtime.cpp
  qml.qrc

  ${DBUS_SOURCES}
)

target_link_libraries(lingmo-filemanager
  PRIVATE
  Qt6::Core
  Qt6::DBus
  Qt6::Quick

  KF6::KIOCore
  KF6::KIOFileWidgets
  KF6::KIOWidgets
  KF6::Solid
  KF6::WindowSystem
  KF6::ConfigCore

  # LingmoUI
)

add_subdirectory(filemanager)
add_subdirectory(dragdrop)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${ts_files_replaced})

install(TARGETS lingmo-filemanager RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES lingmo-filemanager.desktop DESTINATION "/usr/share/applications")
install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-filemanager/translations)
