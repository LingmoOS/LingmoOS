# vim:set softtabstop=3 shiftwidth=3 tabstop=3 expandtab:
project (KActivitiesSLCTestApp)

find_package (Qt6 REQUIRED NO_MODULE COMPONENTS Core Gui Widgets)

if (NOT WIN32)

add_executable(KActivitiesSLCTestApp)

target_include_directories(KActivitiesSLCTestApp PRIVATE
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/autotests/
)

qt_add_dbus_interface(KActivitiesSLCTestApp_DBus_SRCS
   ${KACTIVITIES_CURRENT_ROOT_SOURCE_DIR}/src/service/plugins/slc/org.kde.ActivityManager.SLC.xml
   slc_interface
)

qt_wrap_ui(KActivitiesSLCTestApp_ui_SRCS
   window.ui
)

target_sources(KActivitiesSLCTestApp PRIVATE
   ${KActivitiesSLCTestApp_DBus_SRCS}
   ${KActivitiesSLCTestApp_ui_SRCS}
   window.cpp
   main.cpp
)

target_link_libraries(KActivitiesSLCTestApp
  PRIVATE
      Qt6::Core
      Qt6::Gui
      Qt6::Widgets
      Qt6::DBus
      KF6::Activities
)

endif ()
