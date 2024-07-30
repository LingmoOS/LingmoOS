# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (PlasmaActivitiesStatsTestApp)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets Test Quick QuickWidgets)

if (NOT WIN32)

add_executable (PlasmaActivitiesStatsTestApp)

target_include_directories (PlasmaActivitiesStatsTestApp PRIVATE
   ${KASTATS_CURRENT_ROOT_SOURCE_DIR}/src
   ${KASTATS_CURRENT_ROOT_SOURCE_DIR}/autotests
   ${CMAKE_BINARY_DIR}/src
)

qt_wrap_ui(PlasmaActivitiesStatsTestApp_SRCS
   window.ui
)

qt_add_resources(PlasmaActivitiesStatsTestApp_SRCS
   main.qrc
)

target_sources(PlasmaActivitiesStatsTestApp PRIVATE
   ${PlasmaActivitiesStatsTestApp_SRCS}
   window.cpp
   main.cpp
)

target_link_libraries (PlasmaActivitiesStatsTestApp
   Qt6::Core
   Qt6::Gui
   Qt6::Widgets
   Qt6::Test
   Qt6::Quick
   Qt6::QuickWidgets

   Plasma::Activities
   Plasma::ActivitiesStats
)

endif ()
