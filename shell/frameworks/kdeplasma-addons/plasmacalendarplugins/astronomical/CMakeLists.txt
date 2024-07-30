kcoreaddons_add_plugin(astronomicalevents SOURCES astronomicaleventsplugin.cpp INSTALL_NAMESPACE "plasmacalendarplugins")
target_link_libraries(astronomicalevents
    KF6::ConfigCore
    KF6::Holidays
    KF6::CalendarEvents
    KF6::I18n
)

add_subdirectory(config)
