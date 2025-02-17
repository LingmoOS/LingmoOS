add_executable(kscreenlocker_test kscreenlocker_main.cpp)
target_link_libraries(kscreenlocker_test KScreenLocker Qt::Widgets KF5::I18n)
ecm_mark_as_test(kscreenlocker_test)

add_definitions(-DQML_PATH="${CMAKE_CURRENT_SOURCE_DIR}/powermanagement.qml")
add_executable(powermanagement_test powermanagementtest.cpp ../greeter/powermanagement.cpp)
target_link_libraries(powermanagement_test Qt::Gui Qt::DBus Qt::Quick)
ecm_mark_as_test(powermanagement_test)

add_definitions(-DQML_FILE="${CMAKE_CURRENT_SOURCE_DIR}/kcheckpass_test.qml")
add_executable(kcheckpass_test kcheckpass_test.cpp)
target_link_libraries(kcheckpass_test kscreenlocker_authenticator KF5::CoreAddons Qt::Gui Qt::Quick)
ecm_mark_as_test(kcheckpass_test)
