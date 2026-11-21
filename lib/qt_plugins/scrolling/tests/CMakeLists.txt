find_package(Qt6 REQUIRED COMPONENTS Qml Test)

set(scrolling_test_environment
    QT_QPA_PLATFORM=offscreen
    QT_QUICK_BACKEND=software
)

add_executable(scrolling-quick-test quickscrolling_test.cpp)
target_link_libraries(scrolling-quick-test PRIVATE
    Lingmo::ScrollingQuick
    Qt6::Qml
    Qt6::QuickPrivate
    Qt6::Test
)
add_test(NAME scrolling-quick-test COMMAND scrolling-quick-test)
set_tests_properties(scrolling-quick-test PROPERTIES ENVIRONMENT "${scrolling_test_environment}")

add_executable(scrolling-widgets-test widgetscrolling_test.cpp)
target_link_libraries(scrolling-widgets-test PRIVATE Lingmo::ScrollingWidgets Qt6::Test)
add_test(NAME scrolling-widgets-test COMMAND scrolling-widgets-test)
set_tests_properties(scrolling-widgets-test PROPERTIES ENVIRONMENT "${scrolling_test_environment}")
