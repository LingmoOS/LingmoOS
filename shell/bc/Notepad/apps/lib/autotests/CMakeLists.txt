include(ECMMarkAsTest)

find_package(Qt${QT_MAJOR_VERSION}Test ${QT_MIN_VERSION} QUIET REQUIRED)

macro(kate_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp tests.qrc)

    target_link_libraries(
      ${_testname}
      PRIVATE
        kateprivate
        Qt::Test
    )

    add_test(NAME kateapp-${_testname} COMMAND ${_testname})
    ecm_mark_as_test(${_testname})
  endforeach()
endmacro()

kate_executable_tests(
  session_test
  session_manager_test
  sessions_action_test
  urlinfo_test
  json_utils_test
  location_history_test
  kate_view_mgmt_tests
  bytearraysplitter_tests
)
