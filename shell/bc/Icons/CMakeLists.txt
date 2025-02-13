cmake_minimum_required(VERSION 3.5)
project(cute-icons)

include(FeatureSummary)

install(DIRECTORY Crule DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons)
install(DIRECTORY Crule-dark DESTINATION ${CMAKE_INSTALL_PREFIX}/share/icons)
