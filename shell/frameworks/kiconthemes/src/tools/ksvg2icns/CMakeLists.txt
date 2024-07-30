add_executable(ksvg2icns ksvg2icns.cpp)
ecm_mark_nongui_executable(ksvg2icns)
target_link_libraries(ksvg2icns Qt6::Gui Qt6::Svg)

install(TARGETS ksvg2icns ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
