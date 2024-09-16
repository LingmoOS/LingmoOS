#####################################################################
## dtkdeclarativeprivatesplugin Plugin:
#####################################################################

qt_add_qml_module(dtkdeclarativeprivatesplugin
    PLUGIN_TARGET dtkdeclarativeprivatesplugin
    URI "${URI}.private"
    VERSION "1.0"
    QML_FILES
        "ProgressBarImpl.qml"
        "ProgressBarPanel.qml"
        "ColorOverlay.qml"
        "SoftwareColorOverlay.qml"
        "ArrowListViewButton.qml"
        "ButtonPanel.qml"
        "KeySequenceLabel.qml"
    OUTPUT_DIRECTORY
        "${PLUGIN_OUTPUT_DIR}/${URI_PATH}/private"
)

dtk_extend_target(dtkdeclarativeprivatesplugin EnableCov ${ENABLE_COV})

target_link_libraries(dtkdeclarativeprivatesplugin
PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Quick
)

install(TARGETS dtkdeclarativeprivatesplugin DESTINATION "${QML_INSTALL_DIR}/${URI_PATH}/private")
install(DIRECTORY "${PLUGIN_OUTPUT_DIR}/${URI_PATH}/private/" DESTINATION "${QML_INSTALL_DIR}/${URI_PATH}/private")
