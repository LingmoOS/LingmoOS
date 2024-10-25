cmake_minimum_required(VERSION 3.5)
project(gtk-theme)
install(DIRECTORY Lingmo DESTINATION ${CMAKE_INSTALL_PREFIX}/share/themes)
install(DIRECTORY Lingmo-light DESTINATION ${CMAKE_INSTALL_PREFIX}/share/themes)
install(DIRECTORY Lingmo-dark DESTINATION ${CMAKE_INSTALL_PREFIX}/share/themes)
    
