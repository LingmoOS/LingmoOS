cmake_minimum_required(VERSION 3.5)
project(lingmo-system)

file(GLOB_RECURSE SRCS_R "sources/config/*")
foreach(filepath ${SRCS_R})
    string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
    list(APPEND BASESYSTEM ${filename})
endforeach()
#
# 安装到 ~/.config/
install(FILES ${BASESYSTEM} DESTINATION /System/updates/config/)
install(FILES sources/org.build_lingmo DESTINATION /)

# 安装整个目录及其内容到 ~/.config/
install(DIRECTORY sources/System
        DESTINATION /
        FILES_MATCHING
        PATTERN "*")

install(DIRECTORY sources/desktop/default
        DESTINATION /usr/share/plasma/desktoptheme/
        FILES_MATCHING
        PATTERN "*")

install(DIRECTORY sources/desktop/ocean
        DESTINATION /usr/share/plasma/desktoptheme/
        FILES_MATCHING
        PATTERN "*")

install(DIRECTORY sources/desktop/ocean-dark
        DESTINATION /usr/share/plasma/desktoptheme/
        FILES_MATCHING
        PATTERN "*")

