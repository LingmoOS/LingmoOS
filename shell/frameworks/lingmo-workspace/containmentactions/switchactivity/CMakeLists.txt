kcoreaddons_add_plugin(lingmo_containmentactions_switchactivity SOURCES switch.cpp switch.h INSTALL_NAMESPACE "lingmo/containmentactions")

set_target_properties(lingmo_containmentactions_switchactivity PROPERTIES
    OUTPUT_NAME switchactivity
)

target_link_libraries(lingmo_containmentactions_switchactivity
                      Qt::Widgets
                      Lingmo::Lingmo
                      KF6::KIOCore
                      Lingmo::Activities
                      PW::KWorkspace)
