
add_library(FrameworkIntegrationPlugin
    MODULE frameworkintegrationplugin.cpp)

target_link_libraries(FrameworkIntegrationPlugin
    PRIVATE
        KF6::WidgetsAddons
        KF6::ConfigCore
        KF6::Notifications
)

install(TARGETS FrameworkIntegrationPlugin
        DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6)
