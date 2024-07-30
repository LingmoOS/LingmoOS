ecm_add_qml_module(lingmoextracomponentsplugin VERSION 2.0 URI "org.kde.lingmo.extras" GENERATE_PLUGIN_SOURCE)

target_sources(lingmoextracomponentsplugin PRIVATE
    qmenu.cpp
    qmenuitem.cpp
)

ecm_target_qml_sources(lingmoextracomponentsplugin SOURCES
    qml/ActionTextField.qml
    qml/BasicPlasmoidHeading.qml
    qml/DescriptiveLabel.qml
    qml/ExpandableListItem.qml
    qml/Heading.qml
    qml/Highlight.qml
    qml/ListItem.qml
    qml/ListSectionHeader.qml
    qml/ModelContextMenu.qml
    qml/PasswordField.qml
    qml/PlaceholderMessage.qml
    qml/PlasmoidHeading.qml
    qml/Representation.qml
    qml/SearchField.qml
    qml/ShadowedLabel.qml
)

ecm_target_qml_sources(lingmoextracomponentsplugin PATH animations SOURCES
    qml/animations/ActivateAnimation.qml
    qml/animations/AppearAnimation.qml
    qml/animations/DisappearAnimation.qml
    qml/animations/PressedAnimation.qml
    qml/animations/ReleasedAnimation.qml
)

ecm_target_qml_sources(lingmoextracomponentsplugin PRIVATE PATH private SOURCES
    qml/private/BackgroundMetrics.qml
)

target_link_libraries(lingmoextracomponentsplugin PRIVATE
        Qt6::Quick
        Qt6::Qml
        Qt6::Widgets
        KF6::ConfigWidgets
        Lingmo::Lingmo)

ecm_finalize_qml_module(lingmoextracomponentsplugin DESTINATION ${KDE_INSTALL_QMLDIR})
