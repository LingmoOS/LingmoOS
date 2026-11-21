cmake_minimum_required(VERSION 3.22)

project(cadence VERSION 0.1.4 DESCRIPTION "Frame-driven animation primitives" LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(CTest)

option(CADENCE_BUILD_TESTS "Build Cadence tests" OFF)

add_library(cadence INTERFACE)
add_library(Cadence::Cadence ALIAS cadence)
target_compile_features(cadence INTERFACE cxx_std_20)
target_include_directories(cadence INTERFACE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS cadence EXPORT CadenceTargets)
install(EXPORT CadenceTargets
    FILE CadenceTargets.cmake
    NAMESPACE Cadence::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cadence
)
configure_package_config_file(cmake/CadenceConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/CadenceConfig.cmake
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cadence
)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/CadenceConfigVersion.cmake
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/CadenceConfig.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/CadenceConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Cadence
)

if(CADENCE_BUILD_TESTS)
    find_package(GTest REQUIRED)
    add_executable(cadence-tests tests/animation_test.cpp)
    target_link_libraries(cadence-tests PRIVATE Cadence::Cadence GTest::gtest_main)
    include(GoogleTest)
    gtest_discover_tests(cadence-tests)
endif()
