include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KLanguageButton
    TOOLTIP "Language Button (KF6)"
    WHATSTHIS "A pushbutton for language selection from a popup list."
    GROUP "Buttons (KF6)"
)

ecm_add_qtdesignerplugin(kconfigwidgetswidgets
    NAME KPlottingWidgets
    OUTPUT_NAME kconfigwidgets6widgets
    WIDGETS
        KLanguageButton
    LINK_LIBRARIES
        KF6::ConfigWidgets
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
