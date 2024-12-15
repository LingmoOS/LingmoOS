include_directories(include)
include("${CMAKE_SOURCE_DIR}/modules/compat.cmake")

set(COMPAT_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
include_directories(${COMPAT_ROOT_DIR})

add_subdirectory(protocol)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

add_subdirectory(framework)
add_subdirectory(plugins)

add_subdirectory(apps)
