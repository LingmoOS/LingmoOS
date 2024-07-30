add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_containmentactions_switchwindow\")

set(switchwindow_SRCS
    switch.cpp
    switch.h
)
ki18n_wrap_ui(switchwindow_SRCS config.ui)

kcoreaddons_add_plugin(lingmo_containmentactions_switchwindow SOURCES ${switchwindow_SRCS} INSTALL_NAMESPACE "lingmo/containmentactions")

set_target_properties(lingmo_containmentactions_switchwindow PROPERTIES
    OUTPUT_NAME switchwindow
)

target_link_libraries(lingmo_containmentactions_switchwindow
                      Qt::Widgets
                      Lingmo::Lingmo
                      KF6::I18n
                      PW::LibTaskManager)
