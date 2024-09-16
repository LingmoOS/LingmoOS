cmake_minimum_required(VERSION 3.1.0)

set(CMAKE_BUILD_RPATH ${CMAKE_BINARY_DIR}/src/launcherlib)
set(LAUNCHER "${CMAKE_SOURCE_DIR}/src/launcherlib")
set(COMMON "${CMAKE_SOURCE_DIR}/src/common")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_VERBOSE_MAKEFILE ON)

find_package(Qt5Quick CONFIG REQUIRED)
find_package(DtkDeclarative CONFIG REQUIRED)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${COMMON} ${LAUNCHER}
    )

set(LINK_LIBS
    deepin-turbo
    Qt5::Quick
    ${DtkDeclarative_LIBRARIES}
    )

# Set sources
set(SRC booster-dtkdeclarative.cpp)

# Set executable
add_executable(booster-dtkdeclarative ${SRC} ${MOC_SRC})

target_link_libraries(booster-dtkdeclarative
    ${LINK_LIBS}
    )

# Add install rule
install(TARGETS booster-dtkdeclarative DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/deepin-turbo/)
install(FILES deepin-turbo-booster-dtkdeclarative.service DESTINATION /usr/lib/systemd/user/)
