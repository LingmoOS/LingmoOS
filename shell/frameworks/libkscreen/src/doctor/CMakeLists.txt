add_executable(kscreen-doctor main.cpp doctor.cpp)

target_link_libraries(kscreen-doctor Qt::DBus KF6::Screen KF6::ScreenDpms)
install(TARGETS kscreen-doctor ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES kscreen-doctor.zsh RENAME _kscreen-doctor DESTINATION ${KDE_INSTALL_ZSHAUTOCOMPLETEDIR})
