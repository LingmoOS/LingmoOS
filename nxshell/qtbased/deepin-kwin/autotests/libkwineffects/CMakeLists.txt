include(ECMMarkAsTest)

macro(KWINEFFECTS_UNIT_TESTS)
   foreach(_testname ${ARGN})
      add_executable(${_testname} ${_testname}.cpp)
      add_test(NAME kwineffects-${_testname} COMMAND ${_testname})
      target_link_libraries(${_testname} Qt${QT_MAJOR_VERSION}::Test kwineffects)
      ecm_mark_as_test(${_testname})
   endforeach()
endmacro()

kwineffects_unit_tests(
    windowquadlisttest
    timelinetest
)

add_executable(kwinglplatformtest kwinglplatformtest.cpp mock_gl.cpp ../../src/libkwineffects/kwinglplatform.cpp)
add_test(NAME kwineffects-kwinglplatformtest COMMAND kwinglplatformtest)
target_link_libraries(kwinglplatformtest Qt${QT_MAJOR_VERSION}::Test Qt${QT_MAJOR_VERSION}::Gui KF5::ConfigCore XCB::XCB)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(kwinglplatformtest Qt${QT_MAJOR_VERSION}::X11Extras)
endif()
ecm_mark_as_test(kwinglplatformtest)
