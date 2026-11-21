cmake_minimum_required(VERSION 3.22)
project(lingmo-clock VERSION 1.0.15 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)
find_package(SQLite3 REQUIRED)

add_library(clock-core STATIC
    src/clockcore.cpp
    src/clockstore.cpp
)
target_include_directories(clock-core PUBLIC src)
target_link_libraries(clock-core PUBLIC SQLite::SQLite3)

add_executable(lingmo-clock src/main.cpp src/clockwindow.cpp src/globalmenu.cpp)
target_link_libraries(lingmo-clock PRIVATE clock-core Lingmo::SDK)
target_compile_definitions(lingmo-clock PRIVATE LINGMO_CLOCK_VERSION="${PROJECT_VERSION}")
lingmo_add_translations(lingmo-clock SOURCES src DESKTOP_FILES lingmo-clock.desktop)

add_executable(lingmo-clockd src/clockd.cpp)
target_link_libraries(lingmo-clockd PRIVATE clock-core Lingmo::SDKBase)

install(TARGETS lingmo-clock lingmo-clockd RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-clock.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/org.lingmo.Clock.service DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/services)
install(FILES data/org.lingmo.Clock.xml DESTINATION ${CMAKE_INSTALL_DATADIR}/dbus-1/interfaces)
install(FILES data/lingmo-clockd.service DESTINATION lib/systemd/user)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(clock-core-test tests/clock_core_test.cpp)
    target_link_libraries(clock-core-test PRIVATE clock-core GTest::gtest_main)
    add_test(NAME clock-core COMMAND clock-core-test)
    add_executable(lingmo-clock-capture tests/capture.cpp src/clockwindow.cpp src/globalmenu.cpp)
    target_link_libraries(lingmo-clock-capture PRIVATE clock-core Lingmo::SDK)
    add_test(NAME clock-ui-preview COMMAND lingmo-clock-capture ${CMAKE_CURRENT_BINARY_DIR}/preview)
endif()
