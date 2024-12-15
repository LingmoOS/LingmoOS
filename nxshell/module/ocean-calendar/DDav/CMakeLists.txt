cmake_minimum_required(VERSION 3.7)
project(kdav)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    DBus
REQUIRED)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

#安全编译参数
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fstack-protector-strong -z noexecstack -pie -fPIC -z lazy")

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
aux_source_directory(src KDAV_SRCS)
link_libraries(${Qt5CORE_LIBRARIES} ${Qt5DBus_LIBRARIES})
add_library(${PROJECT_NAME} STATIC ${KDAV_SRCS})


