add_executable(kreadconfig6 kreadconfig.cpp)
ecm_mark_nongui_executable(kreadconfig6)

target_link_libraries(kreadconfig6 KF6::ConfigCore)

install(TARGETS kreadconfig6 ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

add_executable(kwriteconfig6 kwriteconfig.cpp)
ecm_mark_nongui_executable(kwriteconfig6)

target_link_libraries(kwriteconfig6 KF6::ConfigCore)

install(TARGETS kwriteconfig6 ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
