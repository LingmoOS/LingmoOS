
include(ECMQtDeclareLoggingCategory)

add_subdirectory(helper)

set(networkplugin_SRCS
    network.cpp
)

ecm_qt_declare_logging_category(networkplugin_SRCS
    HEADER networklogging.h
    IDENTIFIER KSYSGUARD_PLUGIN_NETWORK
    CATEGORY_NAME org.kde.ksysguard.plugin.network
)

configure_file(networkconstants.h.in networkconstants.h @ONLY)

add_library(ksysguard_plugin_network MODULE ${networkplugin_SRCS})
target_link_libraries(ksysguard_plugin_network Qt::Core Qt::DBus KF6::CoreAddons KF6::I18n KSysGuard::ProcessCore)

install(TARGETS ksysguard_plugin_network DESTINATION ${KDE_INSTALL_PLUGINDIR}/ksysguard/process)
