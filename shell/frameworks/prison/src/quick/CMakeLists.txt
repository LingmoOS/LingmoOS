ecm_add_qml_module(prisonquickplugin URI org.kde.prison VERSION 1.0 DEPENDENCIES QtQuick GENERATE_PLUGIN_SOURCE)

target_sources(prisonquickplugin PRIVATE
    barcodequickitem.cpp
    barcodequickitem.h
)
target_link_libraries(prisonquickplugin PRIVATE Qt6::Quick KF6::Prison)

ecm_finalize_qml_module(prisonquickplugin)
