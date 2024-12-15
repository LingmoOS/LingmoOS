project(lingmo-devicemanager-server C CXX)

add_subdirectory(lingmo-deviceinfo)
add_subdirectory(lingmo-devicecontrol)

#TEST--------------------------------------------------
if (CMAKE_COVERAGE_ARG STREQUAL "CMAKE_COVERAGE_ARG_ON")
   add_subdirectory(./tests)
endif()
