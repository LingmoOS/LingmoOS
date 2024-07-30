add_executable(solid-power main.cpp solid-power.cpp)

ecm_mark_nongui_executable(solid-power)

target_link_libraries(solid-power Qt6::Core KF6::Solid)
install(TARGETS solid-power ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
