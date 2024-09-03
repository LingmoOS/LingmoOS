cmake_minimum_required(VERSION 3.14)

project(lingmo-updator LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Quick DBus LinguistTools REQUIRED)
find_package(QApt REQUIRED)

set(PROJECT_SOURCES
    main.cpp
    updatorhelper.cpp
    upgradeablemodel.cpp
    qml.qrc
)

add_executable(lingmo-updator
  ${PROJECT_SOURCES}
)

target_link_libraries(lingmo-updator
  PRIVATE
  Qt5::Core
  Qt5::Quick
  Qt5::DBus
  QApt::Main
)

file(GLOB TS_FILES translations/*.ts)
qt5_create_translation(QM_FILES ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES} SOURCES ${TS_FILES})
add_dependencies(lingmo-updator translations)

install(TARGETS lingmo-updator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-updator/translations)

install(FILES
    lingmo-updator.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)
