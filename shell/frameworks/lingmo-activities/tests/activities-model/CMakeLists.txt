# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (LingmoActivitiesModelTestApp)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)
find_package (KF6 ${KF6_MIN_VERSION} REQUIRED COMPONENTS WindowSystem)

if (NOT WIN32)

add_executable(LingmoActivitiesModelTestApp)

target_include_directories(LingmoActivitiesModelTestApp PRIVATE
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${LINGMO_ACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
)

qt_wrap_ui(LingmoActivitiesModelTestApp_ui_SRCS
   window.ui
)

target_sources(LingmoActivitiesModelTestApp PRIVATE
   ${LingmoActivitiesModelTestApp_ui_SRCS}
   window.cpp
   window.h
   main.cpp
)

target_link_libraries(LingmoActivitiesModelTestApp
   PRIVATE
      Qt6::Core
      Qt6::Gui
      Qt6::Widgets
      Qt6::DBus
      Lingmo::Activities
      KF6::WindowSystem
)

endif ()
