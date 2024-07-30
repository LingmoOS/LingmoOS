remove_definitions(-DQT_NO_CAST_FROM_ASCII)

include(ECMAddTests)

find_package(Qt6Widgets ${REQUIRED_QT_VERSION} CONFIG QUIET)

if(NOT TARGET Qt6::Test)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

if(NOT Qt6Widgets_FOUND)
    message(STATUS "Qt6Widgets not found, autotests will not be built.")
    return()
endif()

# Include src so we have access to config-kcrash.h
include_directories(${CMAKE_CURRENT_BINARY_DIR}/../src)

# crash_tester enter an infinite loop on Windows (one step from a fork bomb)
# and holds up the CI system infinitely. Disable it in absence of a fix.
if(NOT WIN32)
    ecm_add_tests(
        kcrashtest.cpp
        LINK_LIBRARIES Qt6::Core Qt6::Test
    )

    add_executable(test_crasher test_crasher.cpp)
    target_link_libraries(test_crasher Qt6::Widgets KF6::Crash)
    ecm_mark_as_test(test_crasher)
    ecm_mark_nongui_executable(test_crasher)

    add_dependencies(kcrashtest test_crasher)
endif()

ecm_add_tests(
  coreconfigtest.cpp
  LINK_LIBRARIES Qt6::Core Qt6::Test
)

ecm_add_tests(
  metadatatest.cpp
  LINK_LIBRARIES Qt6::Core Qt6::Test
)
