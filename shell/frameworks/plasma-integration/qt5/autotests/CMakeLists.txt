include(ECMMarkAsTest)
include(ECMMarkNonGuiExecutable)

find_package(Qt5Test ${QT5_MIN_VERSION} CONFIG QUIET)
find_package(Qt5Qml ${QT5_MIN_VERSION} CONFIG QUIET)

if(NOT Qt5Test_FOUND)
    message(STATUS "Qt5Test not found, autotests will not be built.")
    return()
endif()

if(NOT Qt5Qml_FOUND)
    message(STATUS "Qt5Qml not found, QML autotests will not be built.")
endif()

set(CONFIGFILE "${CMAKE_CURRENT_SOURCE_DIR}/kdeplatformtheme_kdeglobals")
set(CHANGED_CONFIGFILE "${CMAKE_CURRENT_SOURCE_DIR}/kdeplatformtheme_changed_kdeglobals")
configure_file(kdeplatformtheme_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/kdeplatformtheme_config.h)

remove_definitions(-DQT_NO_CAST_FROM_ASCII)
# qdbusmenubar uses them
remove_definitions(-DQT_NO_SIGNALS_SLOTS_KEYWORDS)

macro(FRAMEWORKINTEGRATION_TESTS _testname)
    add_executable(${_testname}5 ${_testname}.cpp ${ARGN})
    set_target_properties(${_testname}5 PROPERTIES COMPILE_FLAGS "-DUNIT_TEST")
    add_test(NAME frameworkintegration-${_testname}5 COMMAND ${_testname}5)
    ecm_mark_as_test(${_testname}5)
    ecm_mark_nongui_executable(${_testname}5)
    target_include_directories(${_testname}5 PRIVATE ${CMAKE_BINARY_DIR}/qt5/src/platformtheme)
    target_link_libraries(${_testname}5
        Qt5::GuiPrivate
        Qt5::Test
        Qt5::DBus
        Qt5::QuickControls2
        ${QT5PLATFORMSUPPORT_LIBS}
        KF5::ConfigWidgets
        KF5::ConfigCore
        KF5::IconThemes
        KF5::KIOFileWidgets
        KF5::I18n
        KF5::Notifications
        KF5::WindowSystem
        KF5::GuiAddons
        Qt5::WaylandClient
        XCB::XCB
        Wayland::Client
        KF5::WaylandClient
        KF5::XmlGui
        Qt5::X11Extras
    )
endmacro()

set(dbus_interface)
qt5_add_dbus_interface(dbus_interface ../src/platformtheme/org.kde.StatusNotifierWatcher.xml statusnotifierwatcher_interface)

frameworkintegration_tests(
    kdeplatformtheme_unittest
)
set(wayland_interfaces)
ecm_add_qtwayland_client_protocol(wayland_interfaces
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/appmenu.xml
    BASENAME appmenu
)

ecm_add_qtwayland_client_protocol(wayland_interfaces
    PROTOCOL ${PLASMA_WAYLAND_PROTOCOLS_DIR}/server-decoration-palette.xml
    BASENAME server-decoration-palette
)

target_sources(kdeplatformtheme_unittest5 PRIVATE
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
  ${dbus_interface}
  ${wayland_interfaces}
)
target_sources(kdeplatformtheme_unittest5 PRIVATE
  ../src/platformtheme/qdbusmenubar.cpp # fork of Qt's qdbusmenubar with some added setters for our convenience
)

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

if(Qt5Qml_FOUND)
    add_test(NAME qmltests5 COMMAND qmltestrunner WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

    frameworkintegration_tests(kfiledialogqml_unittest)
    target_link_libraries(kfiledialogqml_unittest5 Qt5::Qml)
endif()
