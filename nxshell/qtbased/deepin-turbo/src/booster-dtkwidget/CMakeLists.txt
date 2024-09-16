cmake_minimum_required(VERSION 3.1.0)

set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")
set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${COMMON} ${LAUNCHER})

# Hide all symbols except the ones explicitly exported in the code (like main())
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_VERBOSE_MAKEFILE ON)

SET(CMAKE_BUILD_RPATH ${CMAKE_BINARY_DIR}/src/launcherlib)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(DtkWidget CONFIG REQUIRED)
find_package(DtkGui CONFIG REQUIRED)

set(LINK_LIBS
    deepin-turbo
    Qt5::Widgets
    ${DtkWidget_LIBRARIES}
    ${DtkGui_LIBRARIES}
    )

# Set sources
set(SRC booster-dtkwidget.cpp)

# Set executable
add_executable(booster-dtkwidget ${SRC} ${MOC_SRC})

target_link_libraries(booster-dtkwidget
    ${LINK_LIBS}
    )

# Add install rule
install(TARGETS booster-dtkwidget DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/deepin-turbo/)
install(FILES deepin-turbo-booster-dtkwidget.service DESTINATION /usr/lib/systemd/user/)
