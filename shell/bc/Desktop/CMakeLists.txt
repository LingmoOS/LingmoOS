cmake_minimum_required(VERSION 3.14)

project(lingmo-desktop LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core DBus Quick LinguistTools REQUIRED)
# find_package(LingmoUI REQUIRED)

find_package(KF5KIO)
find_package(KF5Solid)
find_package(KF5WindowSystem)
find_package(KF5Config)

add_executable(lingmo-desktop
  main.cpp
  desktop.cpp

  desktop/desktopview.cpp
  desktop/dockdbusinterface.cpp
  desktop/desktopsettings.cpp
  thumbnailer/thumbnailprovider.cpp
  thumbnailer/thumbnailcache.cpp
  qml.qrc

  ${DBUS_SOURCES}
)

target_link_libraries(lingmo-desktop
  PRIVATE
  Qt5::Core
  Qt5::DBus
  Qt5::Quick

  KF5::KIOCore
  KF5::KIOFileWidgets
  KF5::KIOWidgets
  KF5::Solid
  KF5::WindowSystem
  KF5::ConfigCore

)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${ts_files_replaced})

install(TARGETS lingmo-desktop RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-desktop/translations)
