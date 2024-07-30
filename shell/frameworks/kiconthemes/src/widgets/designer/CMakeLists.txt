include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KIconButton
    TOOLTIP "Button for selecting an icon (KF6)"
    GROUP "Buttons (KF6)"
)

ecm_add_qtdesignerplugin(kiconthemeswidgets
    NAME KIconThemesWidgets
    OUTPUT_NAME kiconthemes6widgets
    WIDGETS
        KIconButton
    LINK_LIBRARIES
        KF6::IconWidgets
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
