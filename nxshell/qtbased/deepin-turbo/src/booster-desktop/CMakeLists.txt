cmake_minimum_required(VERSION 3.1.0)

set(LAUNCHER "${CMAKE_HOME_DIRECTORY}/src/launcherlib")
set(COMMON "${CMAKE_HOME_DIRECTORY}/src/common")
set(INVOKER "${CMAKE_HOME_DIRECTORY}/src/invoker")
set(CMAKE_CXX_FLGAS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_BUILD_RPATH ${CMAKE_BINARY_DIR}/src/launcherlib)
set(LINK_LIBS deepin-turbo)
set(SRC booster-desktop.cpp ${COMMON}/report.c ${INVOKER}/search.c)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${COMMON} ${LAUNCHER} ${INVOKER})

# Set executable
add_executable(booster-desktop ${SRC})

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

target_link_libraries(booster-desktop
    ${LINK_LIBS}
    Qt5::Widgets
)

# Add install rule
install(TARGETS booster-desktop DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/deepin-turbo/)
install(FILES deepin-turbo-booster-desktop.service DESTINATION /usr/lib/systemd/user/)
