cmake_minimum_required(VERSION 3.7)
project(kcalendarcore)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    DBus
REQUIRED)

#set(LibIcal_MIN_VERSION "3.0")
#find_package(ical ${LibIcal_MIN_VERSION})

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
pkg_check_modules(3rd_lib REQUIRED
        libical
        )

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -z noexecstack -pie -fPIC -z lazy")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src ${3rd_lib_INCLUDE_DIRS})
aux_source_directory(src CALENDARCORE_SRCS)

add_library(${PROJECT_NAME} STATIC ${CALENDARCORE_SRCS})

target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::DBus
    Qt5::Gui
    ${3rd_lib_LIBRARIES}
    ${Libical_LIBRARIES}
    icalvcal
    icalss)
