include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KRichTextEdit
    TOOLTIP "Rich Text Editor (KF6)"
    GROUP "Input (KF6)"
)
ecm_qtdesignerplugin_widget(KRichTextWidget
    TOOLTIP "Rich Text Widget (KF6)"
    GROUP "Input (KF6)"
)
ecm_qtdesignerplugin_widget(KTextEdit
    TOOLTIP "Improved QTextEdit (KF6)"
    WHATSTHIS "An improved version of the QTextEdit with mail or system browser invocation support"
    GROUP "Input (KF6)"
)
ecm_qtdesignerplugin_widget(KPluralHandlingSpinBox
    TOOLTIP "A QSpinBox with plural handling for the suffix (KF6)"
    WHATSTHIS "A QSpinBox with plural handling for the suffix"
    GROUP "Input (KF6)"
)

ecm_add_qtdesignerplugin(ktextwidgetswidgets
    NAME KTextWidgetsWidgets
    OUTPUT_NAME ktextwidgets6widgets
    WIDGETS
        KRichTextEdit
        KRichTextWidget
        KTextEdit
        KPluralHandlingSpinBox
    LINK_LIBRARIES
        KF6::TextWidgets
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
