include(ECMMarkAsTest)

# find_package(Qt5Core ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENT Private _check_private)
include_directories(SYSTEM ${Qt5Core_PRIVATE_INCLUDE_DIRS})
set(testServer_SRCS
    waylandservertest.cpp
    )
add_executable(testServer ${testServer_SRCS})
target_link_libraries(testServer Deepin::DWaylandServer)
ecm_mark_as_test(testServer)

find_package(Qt5Widgets ${QT_MIN_VERSION} CONFIG QUIET)
if (Qt5Widgets_FOUND)
    set(testRenderingServer_SRCS
        renderingservertest.cpp
        )
    add_executable(testRenderingServer ${testRenderingServer_SRCS})
    target_link_libraries(testRenderingServer Deepin::DWaylandServer Qt::Concurrent Qt::Widgets)
    ecm_mark_as_test(testRenderingServer)
endif()

add_executable(copyClient copyclient.cpp)
target_link_libraries(copyClient Deepin::WaylandClient)
ecm_mark_as_test(copyClient)

add_executable(pasteClient pasteclient.cpp)
target_link_libraries(pasteClient Qt::Concurrent Deepin::WaylandClient)
ecm_mark_as_test(pasteClient)

add_executable(clientManagementTest clientManagementTest.cpp)
target_link_libraries(clientManagementTest Deepin::WaylandClient)
ecm_mark_as_test(clientManagementTest)

add_executable(touchClientTest touchclienttest.cpp)
target_link_libraries(touchClientTest Deepin::WaylandClient)

add_executable(panelTest paneltest.cpp)
target_link_libraries(panelTest Deepin::WaylandClient)
ecm_mark_as_test(panelTest)

add_executable(subsurface-test subsurfacetest.cpp)
target_link_libraries(subsurface-test Qt::Core Qt::Gui Deepin::WaylandClient)
ecm_mark_as_test(subsurface-test)

add_executable(shadowTest shadowtest.cpp)
target_link_libraries(shadowTest Deepin::WaylandClient)
ecm_mark_as_test(shadowTest)


if (Qt5Widgets_FOUND)
    add_executable(dpmsTest dpmstest.cpp)
    target_link_libraries(dpmsTest Deepin::WaylandClient Qt::Widgets)
    ecm_mark_as_test(dpmsTest)
endif()

add_executable(plasmasurface-test plasmasurfacetest.cpp)
target_link_libraries(plasmasurface-test Qt::Gui Deepin::WaylandClient)
ecm_mark_as_test(plasmasurface-test)

add_executable(globalproperty-test globalpropertytest.cpp)
target_link_libraries(globalproperty-test Qt::Gui Deepin::WaylandClient)
ecm_mark_as_test(globalproperty-test)

add_executable(xdgforeign-test xdgforeigntest.cpp)
target_link_libraries(xdgforeign-test Qt::Gui Deepin::WaylandClient)
ecm_mark_as_test(xdgforeign-test)

add_executable(xdg-test xdgtest.cpp)
target_link_libraries(xdg-test Qt::Gui Deepin::WaylandClient)
ecm_mark_as_test(xdg-test)

