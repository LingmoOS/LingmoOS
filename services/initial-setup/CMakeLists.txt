cmake_minimum_required(VERSION 3.22)

project(lingmo-initial-setup VERSION 1.3.8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)
find_package(LingmoClients REQUIRED COMPONENTS appearance timedate)

set(INITIAL_SETUP_DATA_DIR ${CMAKE_INSTALL_FULL_DATADIR}/lingmo-initial-setup CACHE PATH
    "Initial setup installed data directory")
set(INITIAL_SETUP_SESSION_DESKTOP_FILE
    ${CMAKE_INSTALL_FULL_DATADIR}/wayland-sessions/lingmo-initial-setup.desktop)
set(INITIAL_SETUP_APPLICATION_DESKTOP_FILE
    ${CMAKE_INSTALL_FULL_DATADIR}/applications/lingmo-initial-setup-app.desktop)

add_executable(lingmo-initial-setup
    src/main.cpp
    src/oobewindow.cpp
)
target_include_directories(lingmo-initial-setup PRIVATE src)
target_compile_definitions(lingmo-initial-setup PRIVATE
    OOBE_DATA_DIR="${INITIAL_SETUP_DATA_DIR}"
    OOBE_SESSION_DESKTOP_FILE="${INITIAL_SETUP_SESSION_DESKTOP_FILE}"
    OOBE_APPLICATION_DESKTOP_FILE="${INITIAL_SETUP_APPLICATION_DESKTOP_FILE}")
target_link_libraries(lingmo-initial-setup PRIVATE Lingmo::SDK Lingmo::AppearanceClient Lingmo::TimeDateClient)

install(TARGETS lingmo-initial-setup RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY data/images/
        DESTINATION ${INITIAL_SETUP_DATA_DIR}/images)

lingmo_add_translations(lingmo-initial-setup SOURCES src
        DESKTOP_FILES data/lingmo-initial-setup.desktop data/lingmo-initial-setup-app.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-initial-setup.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/wayland-sessions)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-initial-setup-app.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

if(BUILD_TESTING)
    add_test(NAME initial-setup-state-test
        COMMAND ${CMAKE_COMMAND} -E env
            PYTHONPATH=${CMAKE_CURRENT_SOURCE_DIR}/tests
            python3 ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_state.py)
endif()
