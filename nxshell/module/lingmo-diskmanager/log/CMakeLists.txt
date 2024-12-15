

#include_directories(${PROJECT_SOURCE_DIR}/log)

#add_library(dlmlog SHARED
#    log.cpp
#    )
#find_package(PkgConfig REQUIRED)
#find_package(DtkGui REQUIRED)
#find_package(Qt5 COMPONENTS
#    Core
#    Widgets
#    DBus
#REQUIRED)

#set(LINK_LIBS
#    Qt5::Core
#    Qt5::DBus
#    ${DtkCore_LIBRARIES}
#    parted
#    parted-fs-resize
#)


#target_link_libraries(dlmlog Qt5::Core)

#install(TARGETS dlmlog DESTINATION lib)

cmake_minimum_required(VERSION 3.0)
project(ddmlog)

# Find the library
find_package(PkgConfig REQUIRED)
find_package(Qt5 COMPONENTS
    Core
    DBus
REQUIRED)


set(CMAKE_CXX_STANDARD 14)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(LOG_SRCS
    log.h
    log.cpp
    )

add_library(${PROJECT_NAME} STATIC ${LOG_SRCS})
target_include_directories(${PROJECT_NAME} PUBLIC
    ${Qt5Gui_PRIVATE_INCLUDE_DIRS}
    ${PROJECT_BINARY_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${Qt5Widgets_LIBRARIES}
    ${Qt5DBus_LIBRARIES}
)

set(CMAKE_INSTALL_PREFIX /usr)
# Install the executable
install(TARGETS ${PROJECT_NAME} DESTINATION lib)

