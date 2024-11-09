cmake_minimum_required(VERSION 3.5)
project(unsplash-wallpapers)

file(GLOB_RECURSE SRCS_R "*.jpg" "*.png")
foreach(filepath ${SRCS_R})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND BACKGROUNDS ${filename})
endforeach()

install(FILES ${BACKGROUNDS} DESTINATION /usr/share/backgrounds/lingmoos)
