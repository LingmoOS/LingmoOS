include(ECMAddQtDesignerPlugin)

ecm_qtdesignerplugin_widget(KCategorizedView
    TOOLTIP "Categorized Item View (KF6)"
    WHATSTHIS "Item view for listing items in a categorized fashion optionally."
    GROUP "Views (KF6)"
)
ecm_qtdesignerplugin_widget(KListWidgetSearchLine
    TOOLTIP "QListWidget Search Line (KF6)"
    GROUP "Input (KF6)"
)
ecm_qtdesignerplugin_widget(KTreeWidgetSearchLine
    TOOLTIP "QTreeWidget Search Line (KF6)"
    GROUP "Input (KF6)"
)
ecm_qtdesignerplugin_widget(KTreeWidgetSearchLineWidget
    TOOLTIP "QTreeWidget Search Line Widget (KF6)"
    GROUP "Input (KF6)"
)

ecm_add_qtdesignerplugin(kitemviewswidgets
    NAME KItemViewsWidgets
    OUTPUT_NAME kitemviews6widgets
    WIDGETS
        KCategorizedView
        KListWidgetSearchLine
        KTreeWidgetSearchLine
        KTreeWidgetSearchLineWidget
    LINK_LIBRARIES
        KF6::ItemViews
    INSTALL_DESTINATION "${KDE_INSTALL_QTPLUGINDIR}/designer"
    COMPONENT Devel
)
