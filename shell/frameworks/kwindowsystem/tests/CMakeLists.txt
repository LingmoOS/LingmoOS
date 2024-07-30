remove_definitions(-DQT_NO_CAST_FROM_ASCII)
remove_definitions(-DQT_NO_CAST_TO_ASCII)

include(ECMMarkAsTest)

find_package(Qt6 ${REQUIRED_QT_VERSION} CONFIG REQUIRED Test Widgets)

macro(kwindowsystem_executable_tests)
  foreach(_testname ${ARGN})
    add_executable(${_testname} ${_testname}.cpp)
    target_link_libraries(${_testname} Qt6::Test Qt6::Widgets KF6::WindowSystem)
    ecm_mark_as_test(${_testname})
  endforeach(_testname)
endmacro()

kwindowsystem_executable_tests(
  blurbehindtest
  backgroundcontrasttest
)

if(KWINDOWSYSTEM_X11)
    kwindowsystem_executable_tests(
      icontest
      createpixmapfromhandletest
      setmainwindowtest
    )
    target_link_libraries(icontest Qt6::GuiPrivate)
endif()

if(KWINDOWSYSTEM_WAYLAND)
  kwindowsystem_executable_tests(
    kwaylandextrastest
  )
endif()
