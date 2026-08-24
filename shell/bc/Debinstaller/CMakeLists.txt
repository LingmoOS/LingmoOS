cmake_minimum_required(VERSION 3.14)

project(lingmo-debinstaller LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(Qt5 COMPONENTS Core Widgets Quick LinguistTools REQUIRED)
find_package(QApt REQUIRED)
# find_package(AptPkg REQUIRED)

add_subdirectory(thirdparty/Apt EXCLUDE_FROM_ALL)
include_directories(thirdparty/Apt)

# Find the required Libaries
include_directories(
    ${APTPKG_INCLUDE_DIR})

set(PROJECT_SOURCES
        src/main.cpp
        src/debinstaller.cpp
        src/aptcli.cpp
        qml.qrc
)

add_executable(lingmo-debinstaller
  ${PROJECT_SOURCES}
)

target_link_libraries(lingmo-debinstaller
  PRIVATE
  Qt5::Core
  Qt5::Widgets
  Qt5::Quick
  QApt
  apt-pkg
  apt-private
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})
add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-debinstaller/translations)
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES
    lingmo-debinstaller.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)
