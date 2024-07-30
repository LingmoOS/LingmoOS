add_executable(kiconfinder6 kiconfinder.cpp)
ecm_mark_nongui_executable(kiconfinder6)
target_link_libraries(kiconfinder6 Qt6::Gui KF6IconThemes)

install(TARGETS kiconfinder6 ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
