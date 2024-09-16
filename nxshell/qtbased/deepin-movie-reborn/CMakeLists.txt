cmake_minimum_required(VERSION 3.0)

project(deepin-movie-reborn)


option(USE_DXCB "integration with dxcb platform plugin" OFF)
option(DMR_DEBUG "turn on debug output" off)
option(DTK_DMAN_PORTAL "turn on dman portal support" off)

execute_process(COMMAND uname -m OUTPUT_VARIABLE MACH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(DMR_DEBUG on)
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_EXPORT_COMPILE_COMMANDS on)

find_package(PkgConfig REQUIRED)

pkg_check_modules(MPRIS REQUIRED IMPORTED_TARGET mpris-qt5)

# test deepin fork mpris-qt5
try_compile(USE_FORK_MPRIS
              ${CMAKE_CURRENT_BINARY_DIR}/try_compile
            SOURCES
              ${CMAKE_CURRENT_SOURCE_DIR}/tests/mpris_test.cc
            LINK_LIBRARIES
              PkgConfig::MPRIS
            OUTPUT_VARIABLE TRY_COMPILE_OUTPUT
)

message(STATUS "use deepin fork mpris-qt5: ${USE_FORK_MPRIS}")

if(USE_FORK_MPRIS)
  add_definitions(-DUSE_FORK_MPRIS)
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(${PROJECT_BINARY_DIR})
include_directories(${PROJECT_SOURCE_DIR})

include(GNUInstallDirs)
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
   set(CMAKE_INSTALL_PREFIX /usr)
endif()

configure_file(${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h @ONLY)

add_subdirectory(src)
add_subdirectory(examples/test)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_subdirectory(tests)
endif()
