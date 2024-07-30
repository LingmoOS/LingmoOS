add_executable(kde-inhibit main.cpp)
target_link_libraries(kde-inhibit Qt::Core Qt::DBus KF6::I18n KF6::CoreAddons)
install(TARGETS kde-inhibit ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES completions/kde-inhibit.zsh RENAME _kde-inhibit DESTINATION ${KDE_INSTALL_ZSHAUTOCOMPLETEDIR})
