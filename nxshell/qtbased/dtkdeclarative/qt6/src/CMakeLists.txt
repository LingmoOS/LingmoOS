include(${PROJECT_SOURCE_DIR}/src/targets.cmake)

find_package(Qt${QT_DEFAULT_MAJOR_VERSION}ShaderTools)

include(${PROJECT_SOURCE_DIR}/qt6/src/qml.cmake)
qt_add_qml_module(${LIB_NAME}
    PLUGIN_TARGET ${PLUGIN_NAME}
    URI ${URI}
    VERSION "1.0"
    SHARED
    NO_GENERATE_PLUGIN_SOURCE
    NO_PLUGIN_OPTIONAL
    QML_FILES
        ${QML_DTK_CONTROLS}
    SOURCES
        ${SRCS} ${HEADERS}
        dquickextendregister_p.h
    OUTPUT_DIRECTORY
        "${PLUGIN_OUTPUT_DIR}/${URI_PATH}"
)

dtk_extend_target(${LIB_NAME} EnableCov ${ENABLE_COV})
dtk_extend_target(${PLUGIN_NAME} EnableCov ${ENABLE_COV})

qt_add_translations(${LIB_NAME}
    TS_FILES ${TS_FILES}
    QM_FILES_OUTPUT_VARIABLE QM_FILES
)

set_target_properties(${LIB_NAME} PROPERTIES
    VERSION ${CMAKE_PROJECT_VERSION}
    SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR}
    EXPORT_NAME Declarative
)

target_link_libraries(${LIB_NAME}
PUBLIC
    ${LIB_NAME}_properties
PRIVATE
    ${LIB_NAME}_sources
)

file(GLOB QML_RCS dtkdeclarative_qml.qrc dtkdeclarative_overridable_qml.qrc)
qt_add_resources(RESOURCES
    ${QML_RCS}
    ${ASSETS_RCS}
)
target_sources(${LIB_NAME} PRIVATE ${RESOURCES})

qt_add_shaders(${LIB_NAME} "_shaders_ng"
    BATCHABLE
    PRECOMPILE
    PREFIX
        "/dtk/declarative"
    FILES
        "shaders_ng/quickitemviewport.vert"
        "shaders_ng/quickitemviewport.frag"
        "shaders_ng/quickitemviewport-opaque.vert"
        "shaders_ng/quickitemviewport-opaque.frag"
        "shaders_ng/cornerscolorshader.vert"
        "shaders_ng/cornerscolorshader.frag"
        "shaders_ng/shadowmaterial.vert"
        "shaders_ng/shadowmaterial.frag"
)

include(${PROJECT_SOURCE_DIR}/qmlplugin/targets.cmake)
target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${PLUGIN_NAME}_interface
    ${LIB_NAME}
)

# Install library
install(TARGETS ${LIB_NAME} EXPORT Dtk${DTK_VERSION_MAJOR}DeclarativeTargets DESTINATION "${LIB_INSTALL_DIR}")
# Install export targets
install(EXPORT Dtk${DTK_VERSION_MAJOR}DeclarativeTargets NAMESPACE Dtk${DTK_VERSION_MAJOR}:: FILE Dtk${DTK_VERSION_MAJOR}DeclarativeTargets.cmake DESTINATION "${CONFIG_INSTALL_DIR}")
# Install translations
install(FILES ${QM_FILES} DESTINATION "${TRANSLATIONS_INSTALL_PATH}")
# Install plugin
install(TARGETS ${PLUGIN_NAME} DESTINATION "${QML_INSTALL_DIR}/${URI_PATH}")
install(DIRECTORY "${PLUGIN_OUTPUT_DIR}/${URI_PATH}/" DESTINATION "${QML_INSTALL_DIR}/${URI_PATH}")
