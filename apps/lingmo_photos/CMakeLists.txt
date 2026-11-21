cmake_minimum_required(VERSION 3.22)
project(lingmo-photos VERSION 1.0.18 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
include(GNUInstallDirs)
include(CTest)

find_package(LingmoSDK REQUIRED)
find_package(SQLite3 REQUIRED)

add_library(photos-core STATIC
    src/photostore.cpp
    src/libraryscanner.cpp
)
target_include_directories(photos-core PUBLIC src)
target_link_libraries(photos-core PUBLIC SQLite::SQLite3 Lingmo::SDKBase)

add_executable(lingmo-photos
    src/main.cpp
    src/photoswindow.cpp
    src/globalmenu.cpp
)
target_link_libraries(lingmo-photos PRIVATE photos-core Lingmo::SDK)
target_compile_definitions(lingmo-photos PRIVATE LINGMO_PHOTOS_VERSION="${PROJECT_VERSION}")
lingmo_add_translations(lingmo-photos SOURCES src DESKTOP_FILES lingmo-photos.desktop)

install(TARGETS lingmo-photos RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lingmo-photos.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)

if(BUILD_TESTING)
    find_package(GTest REQUIRED)
    add_executable(photos-core-test tests/photostore_test.cpp)
    target_link_libraries(photos-core-test PRIVATE photos-core GTest::gtest_main)
    add_test(NAME photos-core COMMAND photos-core-test)
    add_executable(lingmo-photos-capture tests/capture.cpp src/photoswindow.cpp src/globalmenu.cpp)
    target_link_libraries(lingmo-photos-capture PRIVATE photos-core Lingmo::SDK)
    add_test(NAME photos-ui-preview COMMAND lingmo-photos-capture ${CMAKE_CURRENT_BINARY_DIR}/preview)
endif()
