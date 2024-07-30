include(ECMMarkAsTest)

macro(knotifications_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt6::DBus KF6::Notifications)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

if (HAVE_DBUS)
    knotifications_executable_tests(
        unitylaunchertest
        knotificationdbustest
    )
endif()
