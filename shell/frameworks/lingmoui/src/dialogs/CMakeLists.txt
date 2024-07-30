
add_library(LingmoUIDialogs)
ecm_add_qml_module(LingmoUIDialogs URI "org.kde.lingmoui.dialogs"
    VERSION 2.0
    GENERATE_PLUGIN_SOURCE
    DEPENDENCIES QtQuick org.kde.lingmoui.platform
)



ecm_target_qml_sources(LingmoUIDialogs SOURCES
    Dialog.qml
    MenuDialog.qml
    PromptDialog.qml
    SearchDialog.qml
)

set_target_properties(LingmoUIDialogs PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   6
    EXPORT_NAME "LingmoUIDialogs"
)

target_include_directories(LingmoUIDialogs PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/..)

target_link_libraries(LingmoUIDialogs PRIVATE Qt6::Quick LingmoUIPlatform)

include_static_dependencies(LingmoUIDialogs EXPORT LingmoUITargets)

ecm_finalize_qml_module(LingmoUIDialogs)

if (NOT BUILD_SHARED_LIBS)
    install(TARGETS LingmoUIDialogsplugin EXPORT LingmoUITargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

install(TARGETS LingmoUIDialogs EXPORT LingmoUITargets ${KF_INSTALL_DEFAULT_ARGUMENTS})
