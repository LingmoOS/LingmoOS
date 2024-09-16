include(${PROJECT_SOURCE_DIR}/src/targets.cmake)

find_package(Qt${QT_VERSION_MAJOR}QuickCompiler)

# Quick compiler gen qmlc
file(GLOB QML_RCS ${PROJECT_SOURCE_DIR}/src/dtkdeclarative_qml.qrc)
qtquick_compiler_add_resources(RESOURCES
    ${QML_RCS}
)
# Add translation source file
qt5_add_translation(QM_FILES ${TS_FILES})

add_library(${LIB_NAME} SHARED
    ${RESOURCES}
    ${QM_FILES}
)

dtk_extend_target(${LIB_NAME} EnableCov ${ENABLE_COV})

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

# Install translations
install(FILES ${QM_FILES} DESTINATION "${TRANSLATIONS_INSTALL_PATH}")

# Install library
install(TARGETS ${LIB_NAME} EXPORT Dtk${DTK_VERSION_MAJOR}DeclarativeTargets DESTINATION "${LIB_INSTALL_DIR}")
# Install export targets
install(EXPORT Dtk${DTK_VERSION_MAJOR}DeclarativeTargets NAMESPACE Dtk:: FILE Dtk${DTK_VERSION_MAJOR}DeclarativeTargets.cmake DESTINATION "${CONFIG_INSTALL_DIR}")
