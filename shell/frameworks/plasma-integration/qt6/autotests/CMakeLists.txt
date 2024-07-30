include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)

find_package(Qt6Test ${QT_MIN_VERSION} CONFIG QUIET)
find_package(Qt6Qml ${QT_MIN_VERSION} CONFIG QUIET)
find_package(Qt6QmlTools ${QT_MIN_VERSION} CONFIG QUIET) # qmltestrunner

if(NOT Qt6Test_FOUND)
    message(STATUS "Qt6Test not found, autotests will not be built.")
    return()
endif()

if(NOT Qt6Qml_FOUND)
    message(STATUS "Qt6Qml not found, QML autotests will not be built.")
endif()

set(CONFIGFILE "${CMAKE_CURRENT_SOURCE_DIR}/kdeplatformtheme_kdeglobals")
set(CHANGED_CONFIGFILE "${CMAKE_CURRENT_SOURCE_DIR}/kdeplatformtheme_changed_kdeglobals")
configure_file(kdeplatformtheme_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/kdeplatformtheme_config.h)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
# qdbusmenubar uses them
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)

macro(FRAMEWORKINTEGRATION_TESTS _testname)
    add_executable(${_testname}6 ${_testname}.cpp ${ARGN})
    set_target_properties(${_testname}6 PROPERTIES COMPILE_FLAGS "-DUNIT_TEST")
    add_test(NAME frameworkintegration-${_testname}6 COMMAND ${_testname}6)
    ecm_mark_as_test(${_testname}6)
    ecm_mark_nongui_executable(${_testname}6)
    target_include_directories(${_testname}6 PRIVATE ${CMAKE_BINARY_DIR}/qt6/src/platformtheme)
    target_link_libraries(${_testname}6
        Qt6::GuiPrivate
        Qt6::Test
        Qt6::DBus
        Qt6::QuickControls2
        ${QT5PLATFORMSUPPORT_LIBS}
        KF6::ConfigWidgets
        KF6::ConfigCore
        KF6::IconThemes
        KF6::KIOFileWidgets
        KF6::I18n
        KF6::Notifications
        KF6::StatusNotifierItem
        KF6::WindowSystem
        KF6::GuiAddons
        Qt6::WaylandClient
        XCB::XCB
        Wayland::Client
        KF6::XmlGui
    )
    if (QT_MAJOR_VERSION EQUAL "5")
        target_link_libraries(${_testname} Qt5::X11Extras)
    endif()
endmacro()

set(dbus_interface)
qt6_add_dbus_interface(dbus_interface ../src/platformtheme/org.kde.StatusNotifierWatcher.xml statusnotifierwatcher_interface)

frameworkintegration_tests(
    kdeplatformtheme_unittest
)
set(wayland_interfaces)
qt6_generate_wayland_protocol_client_sources(kdeplatformtheme_unittest6 FILES
    ${PLASMA_WAYLAND_PROTOCOLS_DIR}/appmenu.xml
    ${PLASMA_WAYLAND_PROTOCOLS_DIR}/server-decoration-palette.xml
)

target_sources(kdeplatformtheme_unittest6 PRIVATE
  ../src/platformtheme/kdeplatformtheme.cpp ../src/platformtheme/kdeplatformtheme.h
  ../src/platformtheme/kfontsettingsdata.cpp ../src/platformtheme/kfontsettingsdata.h
  ../src/platformtheme/khintssettings.cpp ../src/platformtheme/khintssettings.h
  ../src/platformtheme/kdeplatformfiledialoghelper.cpp ../src/platformtheme/kdeplatformfiledialoghelper.h
  ../src/platformtheme/kdeplatformfiledialogbase.cpp
  ../src/platformtheme/kdeplatformsystemtrayicon.cpp ../src/platformtheme/kdeplatformsystemtrayicon.h
  ../src/platformtheme/kdirselectdialog.cpp
  ../src/platformtheme/kfiletreeview.cpp
  ../src/platformtheme/kwaylandintegration.cpp ../src/platformtheme/kwaylandintegration.h
  ../src/platformtheme/x11integration.cpp ../src/platformtheme/x11integration.h
  ../src/platformtheme/qxdgdesktopportalfiledialog.cpp
  ../src/platformtheme/qdbusmenubarwrapper.h
  ../src/platformtheme/kiodelegate.cpp
  ../src/platformtheme/kioopenwith.cpp
  ${dbus_interface}
  ${wayland_interfaces}
)
# TODO KF6 Port D-Bus menu support
target_compile_definitions(kdeplatformtheme_unittest6 PRIVATE -DKF6_TODO_DBUS_MENUBAR)

frameworkintegration_tests(
  kfontsettingsdata_unittest
  ../src/platformtheme/kfontsettingsdata.cpp ../src/platformtheme/kfontsettingsdata.h
)

frameworkintegration_tests(
  kfiledialog_unittest
)

frameworkintegration_tests(
  ksni_unittest
)

frameworkintegration_tests(
  kdeplatformsystemtrayicon_unittest
  ../src/platformtheme/kdeplatformsystemtrayicon.cpp ../src/platformtheme/kdeplatformsystemtrayicon.h
  ${dbus_interface}
)

frameworkintegration_tests(
  kdirselectdialog_unittest
  ../src/platformtheme/kdeplatformfiledialogbase.cpp
  ../src/platformtheme/kdirselectdialog.cpp
  ../src/platformtheme/kfiletreeview.cpp
)

frameworkintegration_tests(
  khintssettings_unittest
  ../src/platformtheme/khintssettings.cpp ../src/platformtheme/khintssettings.h
)

if(Qt6Qml_FOUND AND TARGET Qt6::qmltestrunner)
    add_test(NAME qmltests6 COMMAND Qt6::qmltestrunner WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    frameworkintegration_tests(kfiledialogqml_unittest)
    target_link_libraries(kfiledialogqml_unittest6 Qt6::Qml)
endif()
