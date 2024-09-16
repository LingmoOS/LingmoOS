set(BIN_NAME "ut-ddeam")

include(FindGTest)
find_package(GTest REQUIRED)

add_compile_definitions(DDEAM_UNIT_TESTING)

file(GLOB_RECURSE TESTS ${CMAKE_CURRENT_LIST_DIR}/*.cpp)

add_executable(${BIN_NAME} ${TESTS})

target_include_directories(${BIN_NAME} PRIVATE
    ${PROJECT_BINARY_DIR}/
)

target_link_libraries(${BIN_NAME} PRIVATE
    GTest::gtest
    dde_am_static
)

target_compile_options(${BIN_NAME} PRIVATE
    -fno-access-control
)

include(GoogleTest)
gtest_discover_tests(${BIN_NAME} WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/tests)
