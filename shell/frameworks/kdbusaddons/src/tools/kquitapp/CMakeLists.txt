add_executable(kquitapp6 kquitapp.cpp)
ecm_mark_nongui_executable(kquitapp6)
target_link_libraries(kquitapp6 Qt6::DBus)
install(TARGETS kquitapp6 ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
