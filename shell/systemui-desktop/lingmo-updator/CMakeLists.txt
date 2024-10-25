cmake_minimum_required(VERSION 3.14)

project(lingmo-updator LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# find_package(Qt5 COMPONENTS Core Quick DBus LinguistTools WebEngine WebEngineWidgets REQUIRED)
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
  # Qt5::WebEngine
  # Qt5::WebEngineWidgets
)

file(GLOB TS_FILES translations/*.ts)
foreach(filepath ${TS_FILES})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND ts_files_replaced ${filename})
endforeach()
qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR} ${ts_files_replaced})

add_custom_target(translations ALL DEPENDS ${QM_FILES} SOURCES ${TS_FILES})

foreach(_ts_file ${TS_FILES})
    execute_process(
        COMMAND ${LUPDATE_EXECUTABLE} -recursive ${CMAKE_SOURCE_DIR} -ts ${_ts_file})
    execute_process(COMMAND lrelease ${_ts_file})
endforeach()

install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATADIR}/${PROJECT_NAME}/translations)
install(TARGETS lingmo-updator RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
# install(FILES ${QM_FILES} DESTINATION /usr/share/lingmo-updator/translations)

install(FILES
    lingmo-updator.desktop
    DESTINATION /usr/share/applications/
    COMPONENT Runtime
)
