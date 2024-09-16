cmake_minimum_required(VERSION 3.9)
project(dareader VERSION 1.0.1 DESCRIPTION "library for reading data from fd provided by deepin-authentication")
include(GNUInstallDirs)
add_library(dareader SHARED src/reader.cpp)
set_target_properties(dareader PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER include/dareader/reader.h)
configure_file(dareader.pc.in dareader.pc @ONLY)
target_include_directories(dareader PUBLIC include)
install(TARGETS dareader 
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/dareader)
install(FILES ${CMAKE_BINARY_DIR}/dareader.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
