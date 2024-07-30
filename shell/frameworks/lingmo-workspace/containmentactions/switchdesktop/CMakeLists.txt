kcoreaddons_add_plugin(lingmo_containmentactions_switchdesktop SOURCES desktop.cpp desktop.h INSTALL_NAMESPACE "lingmo/containmentactions")

set_target_properties(lingmo_containmentactions_switchdesktop PROPERTIES
    OUTPUT_NAME org.kde.switchdesktop
)

target_link_libraries(lingmo_containmentactions_switchdesktop
                        Qt::Widgets
                        Lingmo::Lingmo
                        PW::LibTaskManager
                       )

