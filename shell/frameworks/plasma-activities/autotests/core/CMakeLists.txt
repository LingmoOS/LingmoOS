# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (PlasmaActivitiesTest)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Test Core DBus)

if (NOT WIN32)

add_executable(PlasmaActivitiesTest)

target_include_directories(PlasmaActivitiesTest PRIVATE
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/tests/
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   ${CMAKE_BINARY_DIR}/src/
)

target_sources(PlasmaActivitiesTest PRIVATE
   main.cpp
   Process.cpp
   OfflineTest.cpp
   CleanOnlineTest.cpp
   Process.h
   OfflineTest.h
   CleanOnlineTest.h
   ${PLASMA_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/common/test.cpp
)

target_link_libraries(PlasmaActivitiesTest
   PRIVATE
      Qt6::Core
      Qt6::Test
      Qt6::DBus
      Plasma::Activities
)

endif ()
