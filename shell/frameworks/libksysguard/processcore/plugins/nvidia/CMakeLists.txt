add_library(ksysguard_plugin_nvidia MODULE nvidia.cpp nvidia.h)
target_link_libraries(ksysguard_plugin_nvidia Qt::Core KSysGuard::ProcessCore KF6::I18n KF6::CoreAddons)
install(TARGETS ksysguard_plugin_nvidia DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard/process)
