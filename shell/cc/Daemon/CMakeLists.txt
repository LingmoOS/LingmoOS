cmake_minimum_required(VERSION 3.5)

project(lingmo-daemon LANGUAGES CXX)
set(TARGET lingmo-daemon)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 COMPONENTS Core Widgets DBus REQUIRED)
find_package(QApt REQUIRED)

qt5_add_dbus_adaptor(DBUS_SOURCES
                     src/com.lingmo.AppManager.xml
                     src/appmanager.h AppManager)

set(PROJECT_SOURCES
        src/main.cpp
        src/application.cpp
        src/appmanager.cpp
)

set_source_files_properties(${DBUS_SOURCES} PROPERTIES SKIP_AUTOGEN ON)

add_executable(lingmo-daemon
    ${PROJECT_SOURCES}
    ${DBUS_SOURCES}
)

target_link_libraries(lingmo-daemon PRIVATE
    Qt5::Widgets
    Qt5::Core
    Qt5::DBus
    QApt::Main
)

# install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_BINDIR})

install(TARGETS ${TARGET} DESTINATION "/usr/bin/")

install(FILES src/com.lingmo.Daemon.conf DESTINATION "/etc/dbus-1/system.d/")
install(FILES src/com.lingmo.Daemon.service DESTINATION "/usr/share/dbus-1/system-services/")
install(FILES src/lingmo-permission-surveillance DESTINATION "/usr/bin/")
