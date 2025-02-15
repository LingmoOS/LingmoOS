include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KLanguageButton
    TOOLTIP "Language Button (KF5)"
    WHATSTHIS "A pushbutton for language selection from a popup list."
    GROUP "Buttons (KF5)"
)

ecm_add_qtdesignerplugin(kconfigwidgetswidgets
    NAME KPlottingWidgets
    OUTPUT_NAME kconfigwidgets5widgets
    WIDGETS
        KLanguageButton
    LINK_LIBRARIES
        KF5::ConfigWidgets
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
