
set(CMAKE_AUTOMOC ON)

aux_source_directory(. SOURCE)
find_package(Qt5 COMPONENTS Core DBus REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5DBus_INCLUDE_DIRS})

add_executable(lingmosdk-net ${SOURCE})
target_link_libraries(lingmosdk-net Qt5::Core Qt5::DBus)

