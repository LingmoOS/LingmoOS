project(lingmo-notebook-server)

include(${PROJECT_SOURCE_DIR}/../cmake/common.cmake)

execute_process(COMMAND qdbusxml2cpp ${PROJECT_SOURCE_DIR}/../data/org.lingmo.note.xml -a ${PROJECT_SOURCE_DIR}/note_adaptor)
execute_process(COMMAND qdbusxml2cpp ${PROJECT_SOURCE_DIR}/../data/org.lingmo.note.xml -p ${PROJECT_SOURCE_DIR}/note_interface)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)


find_package(QT NAMES Qt6 Qt5)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets LinguistTools Network DBus Sql Concurrent REQUIRED)

#add include 
include_directories(${PROJECT_SOURCE_DIR})

file(GLOB SRC 
    "${PROJECT_SOURCE_DIR}/*.cpp"
    "${PROJECT_SOURCE_DIR}/*.h"
)

add_executable(${PROJECT_NAME}
    ${SRC}
)

target_link_libraries(${PROJECT_NAME} Qt${QT_VERSION_MAJOR}::Widgets
                                      Qt${QT_VERSION_MAJOR}::Network
                                      Qt${QT_VERSION_MAJOR}::DBus
                                      Qt${QT_VERSION_MAJOR}::Sql
                                      Qt${QT_VERSION_MAJOR}::Concurrent)

#dependencies
target_link_libraries(${PROJECT_NAME} "-lpthread -lz")

target_link_lingmosdk_libraries(${PROJECT_NAME})
target_link_KWindowSystem_libraries(${PROJECT_NAME})
target_link_qgsettings_libraries(${PROJECT_NAME})

########### install ###############
install(TARGETS ${PROJECT_NAME} DESTINATION /usr/bin/)
install(FILES ${PROJECT_SOURCE_DIR}/../data/org.lingmo.note.conf DESTINATION /etc/dbus-1/system.d)
install(FILES ${PROJECT_SOURCE_DIR}/../data/org.lingmo.note.service DESTINATION /usr/share/dbus-1/services/)
