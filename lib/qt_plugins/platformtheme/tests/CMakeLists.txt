find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(platformtheme-test
    platformtheme_test.cpp
    ../filedialoghelper.cpp
    ../hintsettings.cpp
    ../waylandexport.cpp
)
target_include_directories(platformtheme-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(platformtheme-test PRIVATE
    Qt6::GuiPrivate
    Qt6::DBus
    Qt6::Test
)
add_test(NAME platformtheme-test COMMAND platformtheme-test)
set_tests_properties(platformtheme-test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
