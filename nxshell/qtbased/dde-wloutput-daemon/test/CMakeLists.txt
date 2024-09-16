
project(dde-wloutput-daemon-test)

set(TARGET_NAME dde-wloutput-daemon-test)

FILE(GLOB SC_FILES "*.cpp" "*.h")

add_executable(${TARGET_NAME} ${SC_FILES})

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    DBus
REQUIRED)

find_package(ECM REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
find_package(DWayland)

target_link_libraries(${TARGET_NAME} Qt5::Core Qt5::DBus DWaylandClient)
