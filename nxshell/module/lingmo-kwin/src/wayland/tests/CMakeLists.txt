include(ECMMarkAsTest)

# find_package(Qt5Core ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENT Private _check_private)
include_directories(SYSTEM ${Qt5Core_PRIVATE_INCLUDE_DIRS})
set(testServer_SRCS
    waylandservertest.cpp
    fakeoutput.cpp
    )
add_executable(testServer ${testServer_SRCS})
target_link_libraries(testServer kwin Qt${QT_MAJOR_VERSION}::CorePrivate)
ecm_mark_as_test(testServer)

find_package(Qt${QT_MAJOR_VERSION}Widgets ${QT_MIN_VERSION} CONFIG QUIET)
if (TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    set(testRenderingServer_SRCS
        renderingservertest.cpp
        fakeoutput.cpp
        )
    add_executable(testRenderingServer ${testRenderingServer_SRCS})
    target_link_libraries(testRenderingServer kwin Qt${QT_MAJOR_VERSION}::Concurrent Qt${QT_MAJOR_VERSION}::Widgets)
    ecm_mark_as_test(testRenderingServer)
endif()

add_executable(copyClient copyclient.cpp)
target_link_libraries(copyClient KF5::WaylandClient)
ecm_mark_as_test(copyClient)

add_executable(pasteClient pasteclient.cpp)
target_link_libraries(pasteClient Qt${QT_MAJOR_VERSION}::Concurrent KF5::WaylandClient)
ecm_mark_as_test(pasteClient)

add_executable(touchClientTest touchclienttest.cpp)
target_link_libraries(touchClientTest KF5::WaylandClient)

add_executable(panelTest paneltest.cpp)
target_link_libraries(panelTest KF5::WaylandClient)
ecm_mark_as_test(panelTest)

add_executable(subsurface-test subsurfacetest.cpp)
target_link_libraries(subsurface-test Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Gui KF5::WaylandClient)
ecm_mark_as_test(subsurface-test)

add_executable(shadowTest shadowtest.cpp)
target_link_libraries(shadowTest KF5::WaylandClient)
ecm_mark_as_test(shadowTest)


if (TARGET Qt${QT_MAJOR_VERSION}::Widgets)
    add_executable(dpmsTest dpmstest.cpp)
    target_link_libraries(dpmsTest KF5::WaylandClient Qt${QT_MAJOR_VERSION}::Widgets)
    ecm_mark_as_test(dpmsTest)
endif()

add_executable(plasmasurface-test plasmasurfacetest.cpp)
target_link_libraries(plasmasurface-test Qt${QT_MAJOR_VERSION}::Gui KF5::WaylandClient)
ecm_mark_as_test(plasmasurface-test)

add_executable(xdgforeign-test xdgforeigntest.cpp)
target_link_libraries(xdgforeign-test Qt${QT_MAJOR_VERSION}::Gui KF5::WaylandClient)
ecm_mark_as_test(xdgforeign-test)

add_executable(xdg-test xdgtest.cpp)
target_link_libraries(xdg-test Qt${QT_MAJOR_VERSION}::Gui KF5::WaylandClient)
ecm_mark_as_test(xdg-test)

