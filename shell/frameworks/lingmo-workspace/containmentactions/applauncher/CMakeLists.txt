add_definitions(-DTRANSLATION_DOMAIN=\"lingmo_containmentactions_applauncher\")

set(applauncher_SRCS
    launch.cpp
)
ki18n_wrap_ui(applauncher_SRCS config.ui)

kcoreaddons_add_plugin(lingmo_containmentactions_applauncher SOURCES ${applauncher_SRCS} INSTALL_NAMESPACE "lingmo/containmentactions")

set_target_properties(lingmo_containmentactions_applauncher PROPERTIES
    OUTPUT_NAME org.kde.applauncher
)

target_link_libraries(lingmo_containmentactions_applauncher Lingmo::Lingmo KF6::KIOCore KF6::KIOWidgets KF6::I18n)
