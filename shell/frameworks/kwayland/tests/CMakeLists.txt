include(ECMMarkAsTest)

add_executable(copyClient copyclient.cpp)
target_link_libraries(copyClient KWaylandClient)
ecm_mark_as_test(copyClient)

add_executable(pasteClient pasteclient.cpp)
target_link_libraries(pasteClient Qt6::Concurrent KWaylandClient)
ecm_mark_as_test(pasteClient)

add_executable(touchClientTest touchclienttest.cpp)
target_link_libraries(touchClientTest KWaylandClient)

add_executable(panelTest paneltest.cpp)
target_link_libraries(panelTest KWaylandClient)
ecm_mark_as_test(panelTest)

add_executable(qtwayland-integration-test qtwaylandintegrationtest.cpp)
target_link_libraries(qtwayland-integration-test Qt6::Core Qt6::Gui KWaylandClient)
ecm_mark_as_test(qtwayland-integration-test)

add_executable(subsurface-test subsurfacetest.cpp)
target_link_libraries(subsurface-test Qt6::Core Qt6::Gui KWaylandClient)
ecm_mark_as_test(subsurface-test)

add_executable(shadowTest shadowtest.cpp)
target_link_libraries(shadowTest KWaylandClient)
ecm_mark_as_test(shadowTest)


if (TARGET Qt6::Widgets)
    add_executable(dpmsTest dpmstest.cpp)
    target_link_libraries(dpmsTest KWaylandClient Qt6::Widgets)
    ecm_mark_as_test(dpmsTest)
endif()

add_executable(plasmasurface-test plasmasurfacetest.cpp)
target_link_libraries(plasmasurface-test Qt6::Gui KWaylandClient)
ecm_mark_as_test(plasmasurface-test)

add_executable(xdgforeign-test xdgforeigntest.cpp)
target_link_libraries(xdgforeign-test Qt6::Gui KWaylandClient)
ecm_mark_as_test(xdgforeign-test)

add_executable(xdg-test xdgtest.cpp)
target_link_libraries(xdg-test Qt6::Gui KWaylandClient)
ecm_mark_as_test(xdg-test)

