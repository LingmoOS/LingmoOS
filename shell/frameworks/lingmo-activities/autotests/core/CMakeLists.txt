# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (LingmoActivitiesTest)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Test Core DBus)

if (NOT WIN32)

add_executable(LingmoActivitiesTest)

target_include_directories(LingmoActivitiesTest PRIVATE
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/tests/
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
   ${CMAKE_BINARY_DIR}/src/
)

target_sources(LingmoActivitiesTest PRIVATE
   main.cpp
   Process.cpp
   OfflineTest.cpp
   CleanOnlineTest.cpp
   Process.h
   OfflineTest.h
   CleanOnlineTest.h
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/common/test.cpp
)

target_link_libraries(LingmoActivitiesTest
   PRIVATE
      Qt6::Core
      Qt6::Test
      Qt6::DBus
      Lingmo::Activities
)

endif ()
