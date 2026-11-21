cmake_minimum_required(VERSION 3.16)

project(lingmo-terminal VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ICU REQUIRED IMPORTED_TARGET icu-uc)

add_subdirectory(qmltermwidget)

add_executable(${PROJECT_NAME}
    src/findbar.cpp
    src/globalmenu.cpp
    src/main.cpp
    src/processhelper.cpp
    src/settingsdialog.cpp
    src/terminalpage.cpp
    src/terminalsettings.cpp
    src/terminalwindow.cpp
    src/windows.cpp
)
target_include_directories(${PROJECT_NAME} PRIVATE src)
target_link_libraries(${PROJECT_NAME} PRIVATE terminal-view)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

lingmo_add_translations(lingmo-terminal SOURCES src qmltermwidget/lib DESKTOP_FILES lingmo-terminal.desktop)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-terminal.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
