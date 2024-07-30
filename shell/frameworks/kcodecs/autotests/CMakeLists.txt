include(ECMAddTests)

find_package(Qt6Test ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt6::Test)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

ecm_add_tests(
    kencodingprobertest.cpp
    rfc2047test.cpp
    base45test.cpp
    codectest.cpp
    kemailaddresstest.cpp
    LINK_LIBRARIES KF6::Codecs Qt6::Test
)

ecm_add_test(
    kcharsetstest.cpp
    LINK_LIBRARIES KF6::Codecs Qt6::Test
)

# Benchmark, compiled, but not run automatically with ctest
add_executable(base64benchmark base64benchmark.cpp)
target_link_libraries(base64benchmark KF6::Codecs Qt6::Test)
