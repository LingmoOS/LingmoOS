add_executable(kreadconfig5 kreadconfig.cpp)
ecm_mark_nongui_executable(kreadconfig5)

target_link_libraries(kreadconfig5 KF5::ConfigCore)

install(TARGETS kreadconfig5 ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

add_executable(kwriteconfig5 kwriteconfig.cpp)
ecm_mark_nongui_executable(kwriteconfig5)

target_link_libraries(kwriteconfig5 KF5::ConfigCore)

install(TARGETS kwriteconfig5 ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
