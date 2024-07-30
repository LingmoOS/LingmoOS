include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KKeySequenceWidget
    TOOLTIP "QKeySequence picker widget. (KF6)"
    WHATSTHIS "A widget to pick a QKeySequence."
    GROUP "Buttons (KF6)"
)
ecm_qtdesignerplugin_widget(KShortcutsEditor
    TOOLTIP "Shortcuts editor widget. (KF6)"
    WHATSTHIS "A widget to edit a list of shortcuts."
    GROUP "Input (KF6)"
)
ecm_qtdesignerplugin_widget(KShortcutWidget
    TOOLTIP "Shortcut picker widget. (KF6)"
    WHATSTHIS "A widget to pick a KShortcut."
    GROUP "Buttons (KF6)"
)

ecm_add_qtdesignerplugin(kxmlguiwidgets
    NAME KXmlGuiWidgets
    OUTPUT_NAME kxmlgui6widgets
    WIDGETS
        KKeySequenceWidget
        KShortcutsEditor
        KShortcutWidget
    LINK_LIBRARIES
        KF6::XmlGui
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
