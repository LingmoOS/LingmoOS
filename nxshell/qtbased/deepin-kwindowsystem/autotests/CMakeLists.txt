remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DAUTOTEST_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}")

include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt5 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test)

add_subdirectory(helper)

if (NOT APPLE)
    find_package(X11)
    find_package(XCB COMPONENTS XCB KEYSYMS)
endif()

macro(KWINDOWSYSTEM_UNIT_TESTS)
   foreach(_testname ${ARGN})
      set(libs KF5::WindowSystem Qt5::Test Qt5::Widgets Qt5::X11Extras XCB::KEYSYMS)
      if(X11_FOUND)
         list(APPEND libs ${XCB_XCB_LIBRARY})
      endif()
      ecm_add_test(${_testname}.cpp LINK_LIBRARIES ${libs} NAME_PREFIX "kwindowsystem-" GUI)
   endforeach(_testname)
endmacro(KWINDOWSYSTEM_UNIT_TESTS)
macro(KWINDOWSYSTEM_EXECUTABLE_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} KF5::WindowSystem Qt5::Test Qt5::X11Extras XCB::XCB)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()


if(X11_FOUND)
    include_directories(${CMAKE_SOURCE_DIR}/src/platforms/xcb)
    kwindowsystem_unit_tests(
        kmanagerselectiontest
        kstartupinfo_unittest
        kxmessages_unittest
        kkeyserver_x11_unittest
    )

    kwindowsystem_unit_tests(
        kwindoweffectstest
        kwindowinfox11test
        kwindowsystemx11test
        kwindowsystem_threadtest
        netrootinfotestwm
        netwininfotestclient
        netwininfotestwm
        compositingenabled_test
    )
    
    kwindowsystem_executable_tests(
        fixx11h_test
        fixx11h_test2
        dontcrashmapviewport
    )
endif()

ecm_add_test(kwindowsystem_platform_wayland_test.cpp LINK_LIBRARIES KF5::WindowSystem Qt5::Test TEST_NAME kwindowsystemplatformwaylandtest NAME_PREFIX "kwindowsystem-" GUI)
