# SPDX-License-Identifier: BSD-2-Clause

add_library(drkonqi-coredumpexcavator OBJECT coredumpexcavator.cpp automaticcoredumpexcavator.cpp)
target_link_libraries(drkonqi-coredumpexcavator Qt6::Core Qt6::DBus)
target_include_directories(drkonqi-coredumpexcavator PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}>")
