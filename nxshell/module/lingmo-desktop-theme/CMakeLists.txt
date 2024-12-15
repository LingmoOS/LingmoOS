cmake_minimum_required(VERSION 3.11)

message(${CMAKE_CURRENT_SOURCE_DIR})

project(lingmo-desktop-theme)

add_subdirectory("bloom")
add_subdirectory("vintage")
add_subdirectory("flow")
add_subdirectory("hazy-color")
add_subdirectory("organic-glass")
add_subdirectory("macaron")
add_subdirectory("square")
add_subdirectory("nirvana")

file(GLOB SOURCEDIRS "${CMAKE_CURRENT_SOURCE_DIR}/icons/*.dci")
foreach ( val  ${SOURCEDIRS} )
    install(FILES ${val} DESTINATION share/dsg/icons)
endforeach ()
