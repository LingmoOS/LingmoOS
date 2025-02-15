include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KKeySequenceWidget
    TOOLTIP "QKeySequence picker widget. (KF5)"
    WHATSTHIS "A widget to pick a QKeySequence."
    GROUP "Buttons (KF5)"
)
ecm_qtdesignerplugin_widget(KShortcutsEditor
    TOOLTIP "Shortcuts editor widget. (KF5)"
    WHATSTHIS "A widget to edit a list of shortcuts."
    GROUP "Input (KF5)"
)
ecm_qtdesignerplugin_widget(KShortcutWidget
    TOOLTIP "Shortcut picker widget. (KF5)"
    WHATSTHIS "A widget to pick a KShortcut."
    GROUP "Buttons (KF5)"
)

ecm_add_qtdesignerplugin(kxmlguiwidgets
    NAME KXmlGuiWidgets
    OUTPUT_NAME kxmlgui5widgets
    WIDGETS
        KKeySequenceWidget
        KShortcutsEditor
        KShortcutWidget
    LINK_LIBRARIES
        KF5::XmlGui
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
