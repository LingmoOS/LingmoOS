include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KComboBox
    TOOLTIP "Combo Box (KF6)"
    GROUP "Input (KF6)"
)
ecm_qtdesignerplugin_widget(KHistoryComboBox
    TOOLTIP "A combobox for offering a history and completion (KF6)"
    WHATSTHIS "A combobox which implements a history like a unix shell"
    GROUP "Input (KF6)"
)
ecm_qtdesignerplugin_widget(KLineEdit
    TOOLTIP "Line Edit (KF6)"
    GROUP "Input (KF6)"
)

ecm_add_qtdesignerplugin(kcompletionwidgets
    NAME KCompletionWidgets
    OUTPUT_NAME kcompletion6widgets
    WIDGETS
        KComboBox
        KHistoryComboBox
        KLineEdit
    LINK_LIBRARIES
        KF6::Completion
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
