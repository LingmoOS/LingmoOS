cmake_minimum_required(VERSION 3.10)
project(deepin-ocr-plugin-manager)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

set(PROJECT_VERSION 1.0.0)

add_definitions(-DSOURCE_PREFIX="${CMAKE_INSTALL_PREFIX}")

include_directories(.)
include_directories(./paddleocr-ncnn)
include_directories(${CMAKE_INSTALL_PREFIX}/include)

aux_source_directory(. allSource)
aux_source_directory(./paddleocr-ncnn allSource)

add_library(${PROJECT_NAME} SHARED ${allSource})

set_target_properties(${PROJECT_NAME} PROPERTIES VERSION 1.0.0 SOVERSION 1)

find_package(PkgConfig REQUIRED)

find_package(OpenMP REQUIRED)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")

pkg_check_modules(3rd_lib REQUIRED ncnn opencv_mobile)

target_include_directories(${PROJECT_NAME} PUBLIC ${3rd_lib_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${3rd_lib_LIBRARIES} -lstdc++fs -ldl)

#install
include(GNUInstallDirs)

set(ModelDir ${CMAKE_INSTALL_PREFIX}/share/${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(FILES
        deepinocrplugin_p.h
        deepinocrplugin.h
        deepinocrplugindef.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/deepin-ocr-plugin-manager)

install(DIRECTORY ../assets/model DESTINATION ${ModelDir})

configure_file(deepin-ocr-plugin-manager.pc.in ${CMAKE_CURRENT_BINARY_DIR}/deepin-ocr-plugin-manager.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-ocr-plugin-manager.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)
