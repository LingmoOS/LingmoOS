install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kquickcontrols)
install(FILES KeySequenceItem.qml DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kquickcontrols)
install(FILES ColorButton.qml DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/kquickcontrols)

add_subdirectory(private)

