cmake_minimum_required(VERSION 3.5)
project(cursor-themes)

install(DIRECTORY lingmo-light DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons)
install(DIRECTORY lingmo-dark DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons)

install(
    FILES config/lingmo-dark-cursor.theme config/lingmo-light-cursor.theme
    DESTINATION /etc/X11/cursors
)