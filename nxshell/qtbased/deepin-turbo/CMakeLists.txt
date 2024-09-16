project(deepin-turbo)

cmake_minimum_required(VERSION 2.6)
cmake_policy(VERSION 2.6)

#
# NOTE: For verbose build use VERBOSE=1
#

# Default C++-flags. Sub-builds might alter these.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -g -O3 -Wl,--as-needed")
# Default C-flags. Sub-builds might alter these.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -g -O3")

set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# Find libdl
find_library(LIBDL NAMES dl)

if ($ENV{DEBUG_BUILD})
    add_definitions(-DDEBUG_BUILD)
endif ($ENV{DEBUG_BUILD})

# Set the program name defines. Must be at this level due to unit tests.
add_definitions(-DPROG_NAME_INVOKER="deepin-turbo-invoker")
add_definitions(-DPROG_NAME_SINGLE_INSTANCE="deepin-turbo-single-instance")

# applauncherd will try to load single-instance using this path
add_definitions(-DSINGLE_INSTANCE_PATH="${CMAKE_INSTALL_PREFIX}/bin/deepin-turbo-single-instance")

# Disable debug logging, only error and warning messages get logged
# Currently effective only for invoker. Launcher part recognizes --debug
# which enables console echoing and debug messages.
add_definitions(-DDEBUG_LOGGING_DISABLED)

# Build with test coverage switch if BUILD_COVERAGE environment variable is set
if ($ENV{BUILD_COVERAGE})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -DWITH_COVERAGE")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -DWITH_COVERAGE")
    set(CMAKE_LD_FLAGS "${CMAKE_LD_FLAGS} --coverage")
endif ($ENV{BUILD_COVERAGE})

# Sub build: applauncherd
add_subdirectory(src)

# Target for documentation (make doc)
#find_program(Doxygen NAMES doxygen)
#if (EXISTS ${Doxygen})
#    add_custom_target(doc COMMAND cd doc && ${Doxygen} doxygen-impldoc.conf COMMAND cd doc && ${Doxygen} doxygen-userdoc.conf COMMAND cd doc && ${Doxygen} doxygen-mdeclarativecache.conf)
#else (EXISTS ${Doxygen})
#    message(STATUS "Doxygen not found: you're not able to build documentation.")
#endif (EXISTS ${Doxygen})

# Install html documentation
install(DIRECTORY doc/implementation-documentation DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/deepin-turbo OPTIONAL)
install(DIRECTORY doc/user-documentation DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/deepin-turbo OPTIONAL)
install(DIRECTORY doc/mdeclarativecache-documentation DESTINATION ${CMAKE_INSTALL_PREFIX}/share/doc/deepin-turbo OPTIONAL)
