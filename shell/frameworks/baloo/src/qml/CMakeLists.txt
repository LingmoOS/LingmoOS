ecm_add_qml_module(balooplugin URI "org.kde.baloo" VERSION 0.1 DEPENDENCIES QtCore GENERATE_PLUGIN_SOURCE)

target_sources(balooplugin PRIVATE
    queryresultsmodel.cpp
)

target_link_libraries(balooplugin PRIVATE Qt6::Core Qt6::Qml KF6::Baloo)

ecm_finalize_qml_module(balooplugin)

add_subdirectory(experimental)
