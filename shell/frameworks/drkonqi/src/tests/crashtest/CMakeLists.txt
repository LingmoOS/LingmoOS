add_executable(crashtest crashtest.cpp)
ecm_mark_as_test(crashtest)
ecm_mark_nongui_executable(crashtest)
target_link_libraries(crashtest KF6::Crash KF6::CoreAddons Qt::Gui Qt::Concurrent)
