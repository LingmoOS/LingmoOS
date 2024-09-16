if (QT_MAJOR_VERSION EQUAL "5")
    find_package(Qt5 REQUIRED COMPONENTS WaylandClient)
endif()

if (XCB_ICCCM_FOUND)
    set(normalhintsbasesizetest_SRCS normalhintsbasesizetest.cpp)
    add_executable(normalhintsbasesizetest ${normalhintsbasesizetest_SRCS})
    target_link_libraries(normalhintsbasesizetest XCB::XCB XCB::ICCCM KF5::WindowSystem)
endif()

# next target
set(screenedgeshowtest_SRCS screenedgeshowtest.cpp)
add_executable(screenedgeshowtest ${screenedgeshowtest_SRCS})
target_link_libraries(screenedgeshowtest Qt${QT_MAJOR_VERSION}::Widgets KF5::ConfigCore KF5::WindowSystem KF5::WaylandClient ${XCB_XCB_LIBRARY})
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(screenedgeshowtest Qt${QT_MAJOR_VERSION}::X11Extras)
else()
    target_link_libraries(screenedgeshowtest Qt${QT_MAJOR_VERSION}::GuiPrivate)
endif()
target_include_directories(screenedgeshowtest PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(x11shadowreader x11shadowreader.cpp)
target_link_libraries(x11shadowreader XCB::XCB Qt${QT_MAJOR_VERSION}::Widgets KF5::ConfigCore KF5::WindowSystem)
if (QT_MAJOR_VERSION EQUAL "5")
    target_link_libraries(x11shadowreader Qt${QT_MAJOR_VERSION}::X11Extras)
else()
    target_link_libraries(x11shadowreader Qt${QT_MAJOR_VERSION}::GuiPrivate)
endif()
target_include_directories(x11shadowreader PRIVATE ${CMAKE_SOURCE_DIR}/src)

add_executable(pointerconstraints pointerconstraintstest.cpp)
add_definitions(-DDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(pointerconstraints XCB::XCB Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Quick KF5::WaylandClient)

add_executable(pointergestures pointergesturestest.cpp)
add_definitions(-DDIR="${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(pointergestures Qt${QT_MAJOR_VERSION}::Gui Qt${QT_MAJOR_VERSION}::Quick KF5::WaylandClient)

add_executable(cursorhotspottest cursorhotspottest.cpp)
target_link_libraries(cursorhotspottest Qt${QT_MAJOR_VERSION}::Widgets)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

if (QT_MAJOR_VERSION EQUAL "5")
    find_package(Qt${QT_MAJOR_VERSION}WaylandClient ${QT_MIN_VERSION} CONFIG REQUIRED NO_MODULE COMPONENTS Private)
    add_executable(xdgactivationtest-qt5 xdgactivationtest-qt5.cpp)
    target_link_libraries(xdgactivationtest-qt5 Qt${QT_MAJOR_VERSION}::Widgets Qt${QT_MAJOR_VERSION}::WaylandClient Qt${QT_MAJOR_VERSION}::WaylandClientPrivate Wayland::Client)
    ecm_add_qtwayland_client_protocol(xdgactivationtest-qt5
        PROTOCOL ${WaylandProtocols_DATADIR}/staging/xdg-activation/xdg-activation-v1.xml
        BASENAME xdg-activation-v1
    )

    add_executable(lockscreenoverlaytest lockscreenoverlaytest.cpp)
    target_link_libraries(lockscreenoverlaytest Qt${QT_MAJOR_VERSION}::Widgets Qt${QT_MAJOR_VERSION}::WaylandClient Qt${QT_MAJOR_VERSION}::WaylandClientPrivate Wayland::Client KF5::WindowSystem)
    ecm_add_qtwayland_client_protocol(lockscreenoverlaytest
        PROTOCOL ${DEEPIN_WAYLAND_PROTOCOLS_DIR}/kde-lockscreen-overlay-v1.xml
        BASENAME kde-lockscreen-overlay-v1
    )
endif()

if (TARGET Qt6::Gui)
    add_executable(xdgactivationtest-qt6 xdgactivationtest-qt6.cpp)
    target_link_libraries(xdgactivationtest-qt6 Qt6::Widgets)
endif()
