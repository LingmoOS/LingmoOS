include(ECMMarkAsTest)

#######################################
# KillTest
#######################################
add_executable(killTest killtest.cpp)
target_compile_definitions(killTest PRIVATE KSCREENLOCKER_GREET_BIN_REL="kscreenlocker_greet")
add_test(NAME kscreenlocker-killTest COMMAND killTest)
ecm_mark_as_test(killTest)
target_link_libraries(killTest KF5::CoreAddons Qt::Test)
