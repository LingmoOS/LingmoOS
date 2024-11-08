
set(CMAKE_AUTOMOC ON)

aux_source_directory(. SOURCE)
find_package(Qt5 COMPONENTS Core DBus REQUIRED)
include_directories(${Qt5Core_INCLUDE_DIRS})
include_directories(${Qt5DBus_INCLUDE_DIRS})

add_executable(lingmosdk-logrotate ${SOURCE})
target_link_libraries(lingmosdk-logrotate Qt5::Core Qt5::DBus)

add_executable(kylogrotate-test test/kylogrotate-test.c)
target_link_libraries(kylogrotate-test kylog)
