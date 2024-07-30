add_executable(qfiledialogtest5 qfiledialogtest.cpp)
target_link_libraries(qfiledialogtest5 Qt5::Core Qt5::Widgets)

add_executable(windowdecotest5 windowdecotest.cpp)
target_link_libraries(windowdecotest5 Qt5::Core Qt5::Widgets KF5::ConfigWidgets)

add_executable(systraymenuexclusiveactionstest5 systraymenuexclusiveactionstest.cpp)
target_link_libraries(systraymenuexclusiveactionstest5 Qt5::Core Qt5::Widgets)
