project(deepin-devicemanager C CXX)

include(GNUInstallDirs)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set(CMAKE_INSTALL_PREFIX /usr)
endif()

set(DISABLE_DRIVER false CACHE BOOL "disable driver")
if (DISABLE_DRIVER)
    add_definitions(-DDISABLE_DRIVER)
endif()

set(DISABLE_POLKIT false CACHE BOOL "disable polkit debug")
if(CMAKE_BUILD_TYPE  STREQUAL "Debug")
    add_definitions(-DDISABLE_POLKIT)
endif()

add_subdirectory(${CMAKE_SOURCE_DIR}/deepin-devicemanager)
add_subdirectory(${CMAKE_SOURCE_DIR}/deepin-devicemanager-server)

#代码覆盖率开关
if(CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -fprofile-arcs -ftest-coverage")
endif()