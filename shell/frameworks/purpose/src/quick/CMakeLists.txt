ecm_add_qml_module(purposequickplugin URI "org.kde.purpose" VERSION 1.0 GENERATE_PLUGIN_SOURCE)

target_sources(purposequickplugin PRIVATE types.h)

ecm_target_qml_sources(purposequickplugin SOURCES
    AlternativesView.qml
    JobView.qml
)

target_link_libraries(purposequickplugin PRIVATE
    Qt6::Qml
    KF6Purpose
)

ecm_finalize_qml_module(purposequickplugin)
