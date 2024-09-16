set(PLUGIN_NAME ${STYLE_PLUGIN_NAME})

find_package(Dtk${DTK_VERSION_MAJOR}Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Qml
    Quick
    QuickControls2 REQUIRED
)
if(EnableQt5)
    find_package(Qt${QT_VERSION_MAJOR}QuickCompiler)
endif()

set(QML_FILES
    ApplicationWindow.qml
    BusyIndicator.qml
    Button.qml
    CheckBox.qml
    CheckDelegate.qml
    ComboBox.qml
    Control.qml
    DelayButton.qml
    DialogButtonBox.qml
    Dial.qml
    Frame.qml
    GroupBox.qml
    ItemDelegate.qml
    MenuItem.qml
    Menu.qml
    MenuSeparator.qml
    PageIndicator.qml
    Pane.qml
    Popup.qml
    ProgressBar.qml
    RadioButton.qml
    RoundButton.qml
    ScrollBar.qml
    ScrollIndicator.qml
    Slider.qml
    SpinBox.qml
    StackView.qml
    Switch.qml
    TextArea.qml
    TextField.qml
    ToolTip.qml
)
set(SRC_FILES
    qtquickcontrols2chameleonstyleplugin.h qtquickcontrols2chameleonstyleplugin.cpp
)

if(EnableQt5)
    file(GLOB _qml_files ${QML_FILES})
    set(TARGETPATH "QtQuick/Controls.2/Chameleon")
    set(PLUGIN_INSTALL_DIR "${QML_INSTALL_DIR}/${TARGETPATH}")

    qtquick_compiler_add_resources(RESOURCES ${CMAKE_CURRENT_LIST_DIR}/qml.qrc)

    add_library(${PLUGIN_NAME} SHARED
        ${SRC_FILES}
        ${RESOURCES} #   qml.qrc
    )
    set_target_properties(${PLUGIN_NAME} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/Chameleon"
    )

    find_program(COMPILER
        NAMES qmlcachegen
        REQUIRED
    )

    set(QMLC_FILES)
    foreach(QML_FILE ${_qml_files})
        get_filename_component(EXTENSION ${QML_FILE} EXT)
        if(EXTENSION STREQUAL ".qml" OR EXTENSION STREQUAL ".js" OR EXTENSION STREQUAL ".ui.qml")
            set(OUTPUT_FILE "${QML_FILE}c")
            execute_process(COMMAND ${COMPILER} ${QML_FILE} -o ${OUTPUT_FILE})
            list(APPEND QMLC_FILES ${OUTPUT_FILE})
        endif()
    endforeach()

    set(QML_DIR "${CMAKE_CURRENT_BINARY_DIR}/qmldir")
    configure_file("${CMAKE_CURRENT_LIST_DIR}/qmldir.in" "${QML_DIR}" @ONLY)
    # copy qmldir
    add_custom_command(TARGET ${PLUGIN_NAME}
       POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E
       copy_if_different
       "${QML_DIR}"    #in-file
       "${PLUGIN_OUTPUT_DIR}/Chameleon/qmldir"                            #out-file
    )

    install(TARGETS ${PLUGIN_NAME} DESTINATION ${PLUGIN_INSTALL_DIR})
    install(FILES
        ${QML_DIR}
        ${_qml_files}
        ${QMLC_FILES}
        DESTINATION ${PLUGIN_INSTALL_DIR}
    )
endif()

if(EnableQt6)
    qt_add_qml_module(${PLUGIN_NAME}
        URI "Chameleon"
        VERSION "${PROJECT_VERSION}"
        CLASS_NAME QtQuickControls2ChameleonStylePlugin
        PLUGIN_TARGET ${PLUGIN_NAME}
        OUTPUT_DIRECTORY
            "${PLUGIN_OUTPUT_DIR}/Chameleon"
        NO_PLUGIN_OPTIONAL
        NO_GENERATE_PLUGIN_SOURCE
        SOURCES ${SRC_FILES}
        QML_FILES ${QML_FILES}
    )
    install(DIRECTORY "${PLUGIN_OUTPUT_DIR}/Chameleon/" DESTINATION "${QML_INSTALL_DIR}/Chameleon")
endif()

dtk_extend_target(${PLUGIN_NAME} EnableCov ${ENABLE_COV})

target_link_libraries(${PLUGIN_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Qml
    Qt${QT_VERSION_MAJOR}::Quick
    Qt${QT_VERSION_MAJOR}::QuickControls2
)

if(USE_QQuickStylePluginPrivate)
    target_compile_definitions(${PLUGIN_NAME} PRIVATE
        USE_QQuickStylePlugin)
    target_link_libraries(${PLUGIN_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::QuickControls2Private
    )
endif()
