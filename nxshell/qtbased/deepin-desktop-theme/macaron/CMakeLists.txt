cmake_minimum_required(VERSION 3.11)

message(${CMAKE_CURRENT_SOURCE_DIR})

string(REGEX REPLACE ".*/(.*)" "\\1" PROJ ${CMAKE_CURRENT_SOURCE_DIR})
project(deepin-theme-${PROJ})

message(${PROJECT_NAME})

macro(install_dir sourcedir desdir)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${sourcedir}")
        file(GLOB SOURCEDIRS "${CMAKE_CURRENT_SOURCE_DIR}/${sourcedir}/*")
        foreach ( val  ${SOURCEDIRS} )
            if (IS_DIRECTORY ${val} )
                install(DIRECTORY ${val} DESTINATION ${desdir})
            else()
                install(FILES ${val} DESTINATION ${desdir})
            endif()
        endforeach ()
    endif()
endmacro()

install_dir(icons share/icons)
install_dir(dsg-icons share/dsg/icons)
install_dir(deepin-themes share/deepin-themes)
#install_dir(wallpapers share/wallpapers/deepin)
