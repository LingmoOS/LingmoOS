remove_definitions(-DQT_NO_CAST_FROM_BYTEARRAY)
remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)
add_definitions(-DAUTOTEST_BUILD_DIR="${CMAKE_CURRENT_BINARY_DIR}")

include(ECMMarkAsTest)
include(ECMAddTests)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Widgets)

add_subdirectory(helper)

macro(KWINDOWSYSTEM_UNIT_TESTS)
   foreach(_testname ${ARGN})
      set(libs KF6::WindowSystem Qt6::Test Qt6::Widgets Qt6::GuiPrivate)

      if (KWINDOWSYSTEM_X11)
         list(APPEND libs XCB::XCB XCB::KEYSYMS XCB::ICCCM)
      endif()

      ecm_add_test(${_testname}.cpp LINK_LIBRARIES ${libs} NAME_PREFIX "kwindowsystem-" GUI)
   endforeach(_testname)
endmacro(KWINDOWSYSTEM_UNIT_TESTS)
macro(KWINDOWSYSTEM_EXECUTABLE_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      target_link_libraries(${_testname} KF6::WindowSystem Qt6::Test XCB::XCB Qt6::GuiPrivate)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

if(KWINDOWSYSTEM_X11)
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

ecm_add_test(kwindowsystem_platform_wayland_test.cpp LINK_LIBRARIES KF6::WindowSystem Qt6::Test TEST_NAME kwindowsystemplatformwaylandtest NAME_PREFIX "kwindowsystem-" GUI)
