ecm_add_tests(
    xdgshortcuttest.cpp

    LINK_LIBRARIES Qt::Test Qt::Gui
)
target_sources(xdgshortcuttest PRIVATE ${CMAKE_SOURCE_DIR}/src/xdgshortcut.cpp ${CMAKE_SOURCE_DIR}/src/xdgshortcut.h)
target_link_libraries(xdgshortcuttest Qt::GuiPrivate)

add_test(
    NAME colorschemetest
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/colorschemetest.py
)
set_tests_properties(colorschemetest PROPERTIES ENVIRONMENT "CMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

ecm_add_test(inputcapturebarriertest.cpp ${CMAKE_SOURCE_DIR}/src/inputcapturebarrier.cpp TEST_NAME inputcapturebarriertest LINK_LIBRARIES Qt::Test Qt::Gui)
