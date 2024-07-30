# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:

string (REPLACE "-fno-exceptions" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_definitions (-fexceptions)

add_subdirectory (core)

