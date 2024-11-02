cmake_minimum_required(VERSION 3.14)

project(lingmo-process-manager-daemon LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core DBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core DBus)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/../base
    ${CMAKE_CURRENT_SOURCE_DIR}/../utils)

set (SRC_Sources
    src/main.cpp
    src/processmanager.cpp
    src/cgroupmanager.cpp
    src/systemdunitmanager.cpp
    src/cpuraplenergymeter.cpp
    src/cgroupv2releasenotification.cpp
)

set (SRC_Headers
    src/processmanager.h
    src/cgroupmanager.h
    src/systemdunitmanager.h
    src/cpuraplenergymeter.h
    src/cgroupv2releasenotification.h
)

qt5_add_dbus_adaptor(SRC_Sources 
    configs/com.lingmo.ProcessManagerDaemon.xml 
    src/processmanager.h 
    ProcessManager
    processmanagerservice
    ProcessManagerService)

add_executable(
    lingmo-process-manager-daemon
    ${BASE_FILE}
    ${SRC_Sources}
    ${SRC_Headers}
    src/resourcewatcher.cpp src/resourcewatcher.h
    src/resource.cpp src/resource.h
    src/pressurewatcher.cpp src/pressurewatcher.h
    src/memorywatcher.cpp src/memorywatcher.h
    src/systemresourcemanager.h src/systemresourcemanager.cpp
)

target_link_libraries(lingmo-process-manager-daemon 
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::DBus
    cgroup
    jsoncpp
    procps
    pthread
)

install(TARGETS lingmo-process-manager-daemon DESTINATION /usr/bin)
install(FILES configs/lingmo-process-manager-thawer DESTINATION /usr/bin)
install(FILES configs/com.lingmo.ProcessManagerDaemon.conf DESTINATION /usr/share/dbus-1/system.d)
install(FILES configs/com.lingmo.ProcessManagerDaemon.service DESTINATION /usr/share/dbus-1/system-services)
install(FILES configs/lingmo-process-manager-daemon.service DESTINATION /lib/systemd/system)
install(FILES configs/lingmo-process-manager-thawer.service DESTINATION /lib/systemd/system)
install(FILES configs/lingmo-process-manager-cleaner DESTINATION /usr/bin)
