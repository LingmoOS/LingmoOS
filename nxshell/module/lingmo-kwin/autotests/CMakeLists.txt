add_definitions(-DKWIN_UNIT_TEST)
remove_definitions(-DQT_USE_QSTRINGBUILDER)
add_subdirectory(libkwineffects)
add_subdirectory(integration)
add_subdirectory(libinput)
add_subdirectory(tabbox)
add_subdirectory(drm)

########################################################
# Test WindowPaintData
########################################################
set(testWindowPaintData_SRCS test_window_paint_data.cpp)
add_executable(testWindowPaintData ${testWindowPaintData_SRCS})
target_link_libraries(testWindowPaintData kwineffects Qt${QT_MAJOR_VERSION}::Widgets Qt${QT_MAJOR_VERSION}::Test )
add_test(NAME kwin-testWindowPaintData COMMAND testWindowPaintData)
ecm_mark_as_test(testWindowPaintData)

########################################################
# Test VirtualDesktopManager
########################################################
set(testVirtualDesktops_SRCS
    ../src/virtualdesktops.cpp
    test_virtual_desktops.cpp
)
add_executable(testVirtualDesktops ${testVirtualDesktops_SRCS})

target_link_libraries(testVirtualDesktops
    kwin

    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets

    KF5::ConfigCore
    KF5::GlobalAccel
    KF5::I18n
    KF5::WindowSystem
)
add_test(NAME kwin-testVirtualDesktops COMMAND testVirtualDesktops)
ecm_mark_as_test(testVirtualDesktops)

########################################################
# Test ClientMachine
########################################################
set(testClientMachine_SRCS
    ../src/client_machine.cpp
    test_client_machine.cpp
    xcb_scaling_mock.cpp
)
add_executable(testClientMachine ${testClientMachine_SRCS})
set_target_properties(testClientMachine PROPERTIES COMPILE_DEFINITIONS "NO_NONE_WINDOW")

target_link_libraries(testClientMachine
    Qt${QT_MAJOR_VERSION}::Concurrent
    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets

    KF5::ConfigCore
    KF5::WindowSystem

    XCB::XCB
    XCB::XFIXES

    ${X11_X11_LIB} # to make jenkins happy
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(testClientMachine Qt${QT_MAJOR_VERSION}::X11Extras)
else()
    target_link_libraries(testClientMachine Qt${QT_MAJOR_VERSION}::GuiPrivate)
endif()
add_test(NAME kwin-testClientMachine COMMAND testClientMachine)
ecm_mark_as_test(testClientMachine)

########################################################
# Test XcbWrapper
########################################################
add_executable(testXcbWrapper test_xcb_wrapper.cpp xcb_scaling_mock.cpp)

target_link_libraries(testXcbWrapper
    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets

    KF5::ConfigCore
    KF5::WindowSystem

    XCB::XCB
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(testXcbWrapper Qt${QT_MAJOR_VERSION}::X11Extras)
else()
    target_link_libraries(testXcbWrapper Qt${QT_MAJOR_VERSION}::GuiPrivate)
endif()
add_test(NAME kwin-testXcbWrapper COMMAND testXcbWrapper)
ecm_mark_as_test(testXcbWrapper)

if (XCB_ICCCM_FOUND)
    add_executable(testXcbSizeHints test_xcb_size_hints.cpp xcb_scaling_mock.cpp)
    set_target_properties(testXcbSizeHints PROPERTIES COMPILE_DEFINITIONS "NO_NONE_WINDOW")
    target_link_libraries(testXcbSizeHints
        Qt${QT_MAJOR_VERSION}::Test
        Qt${QT_MAJOR_VERSION}::Widgets

        KF5::ConfigCore
        KF5::WindowSystem

        XCB::ICCCM
        XCB::XCB
    )
    if (QT_MAJOR_VERSION EQUAL "5")
        target_link_libraries(testXcbSizeHints Qt${QT_MAJOR_VERSION}::X11Extras)
    else()
        target_link_libraries(testXcbSizeHints Qt${QT_MAJOR_VERSION}::GuiPrivate)
    endif()
    add_test(NAME kwin-testXcbSizeHints COMMAND testXcbSizeHints)
    ecm_mark_as_test(testXcbSizeHints)
endif()

########################################################
# Test XcbWindow
########################################################
add_executable(testXcbWindow test_xcb_window.cpp xcb_scaling_mock.cpp)

target_link_libraries(testXcbWindow
    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets

    KF5::ConfigCore
    KF5::WindowSystem

    XCB::XCB
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(testXcbWindow Qt${QT_MAJOR_VERSION}::X11Extras)
else()
    target_link_libraries(testXcbWindow Qt${QT_MAJOR_VERSION}::GuiPrivate)
endif()
add_test(NAME kwin-testXcbWindow COMMAND testXcbWindow)
ecm_mark_as_test(testXcbWindow)

include_directories(${KWin_SOURCE_DIR}/src)

########################################################
# Test OnScreenNotification
########################################################
set(testOnScreenNotification_SRCS
    ../src/input_event_spy.cpp
    ../src/onscreennotification.cpp
    onscreennotificationtest.cpp
)
add_executable(testOnScreenNotification ${testOnScreenNotification_SRCS})

target_link_libraries(testOnScreenNotification
    Qt${QT_MAJOR_VERSION}::DBus
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets # QAction include

    KF5::ConfigCore
)

add_test(NAME kwin-testOnScreenNotification COMMAND testOnScreenNotification)
ecm_mark_as_test(testOnScreenNotification)

########################################################
# Test Gestures
########################################################
set(testGestures_SRCS
    ../src/gestures.cpp
    test_gestures.cpp
)
add_executable(testGestures ${testGestures_SRCS})

target_link_libraries(testGestures
    Qt${QT_MAJOR_VERSION}::Test
)

add_test(NAME kwin-testGestures COMMAND testGestures)
ecm_mark_as_test(testGestures)

########################################################
# Test X11 TimestampUpdate
########################################################
add_executable(testX11TimestampUpdate test_x11_timestamp_update.cpp)
target_link_libraries(testX11TimestampUpdate
    KF5::CoreAddons
    Qt${QT_MAJOR_VERSION}::Test
    kwin
)
add_test(NAME kwin-testX11TimestampUpdate COMMAND testX11TimestampUpdate)
ecm_mark_as_test(testX11TimestampUpdate)

set(testOpenGLContextAttributeBuilder_SRCS
    ../src/utils/abstract_opengl_context_attribute_builder.cpp
    ../src/utils/egl_context_attribute_builder.cpp
    opengl_context_attribute_builder_test.cpp
)

if (HAVE_EPOXY_GLX)
    set(testOpenGLContextAttributeBuilder_SRCS ${testOpenGLContextAttributeBuilder_SRCS} ../src/backends/x11/standalone/x11_standalone_glx_context_attribute_builder.cpp)
endif()
add_executable(testOpenGLContextAttributeBuilder ${testOpenGLContextAttributeBuilder_SRCS})
target_link_libraries(testOpenGLContextAttributeBuilder epoxy::epoxy Qt${QT_MAJOR_VERSION}::Test)
add_test(NAME kwin-testOpenGLContextAttributeBuilder COMMAND testOpenGLContextAttributeBuilder)
ecm_mark_as_test(testOpenGLContextAttributeBuilder)

set(testXkb_SRCS
    ../src/xkb.cpp
    test_xkb.cpp
)
if (QT_MAJOR_VERSION EQUAL "5")
    qt5_add_dbus_interface(testXkb_SRCS ${CMAKE_SOURCE_DIR}/src/org.freedesktop.DBus.Properties.xml dbusproperties_interface)
else()
    qt_add_dbus_interface(testXkb_SRCS ${CMAKE_SOURCE_DIR}/src/org.freedesktop.DBus.Properties.xml dbusproperties_interface)
endif()

add_executable(testXkb ${testXkb_SRCS})
target_link_libraries(testXkb
    kwin

    Qt${QT_MAJOR_VERSION}::Gui
    Qt${QT_MAJOR_VERSION}::Test
    Qt${QT_MAJOR_VERSION}::Widgets
    Qt${QT_MAJOR_VERSION}::DBus

    KF5::ConfigCore
    KF5::WindowSystem

    kwineffects

    XKB::XKB
)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(testXkb Qt5XkbCommonSupport::Qt5XkbCommonSupport)
else()
    target_link_libraries(testXkb Qt${QT_MAJOR_VERSION}::GuiPrivate)
endif()
add_test(NAME kwin-testXkb COMMAND testXkb)
ecm_mark_as_test(testXkb)

########################################################
# Test FTrace
########################################################
add_executable(testFtrace test_ftrace.cpp)
target_link_libraries(testFtrace
    Qt${QT_MAJOR_VERSION}::Test
    kwin
)
add_test(NAME kwin-testFtrace COMMAND testFtrace)
ecm_mark_as_test(testFtrace)

########################################################
# Test KWin Utils
########################################################
add_executable(testUtils test_utils.cpp)
target_link_libraries(testUtils
    Qt${QT_MAJOR_VERSION}::Test
    kwin
)
add_test(NAME kwin-testUtils COMMAND testUtils)
ecm_mark_as_test(testUtils)
