include(${PROJECT_SOURCE_DIR}/qmlplugin/targets.cmake)
add_library(${PLUGIN_NAME} SHARED)

dtk_extend_target(${PLUGIN_NAME} EnableCov ${ENABLE_COV})

target_link_libraries(${PLUGIN_NAME} PRIVATE
    ${PLUGIN_NAME}_interface
    ${LIB_NAME}
)

set_target_properties(${PLUGIN_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PLUGIN_OUTPUT_DIR}/${URI_PATH}"
)

# copy qmldir
add_custom_command(TARGET ${PLUGIN_NAME}
   POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E
   copy_if_different
   "${PROJECT_SOURCE_DIR}/qmlplugin/qmldir"    #in-file
   "${PLUGIN_OUTPUT_DIR}/${URI_PATH}/qmldir"    #out-file
)

install(TARGETS ${PLUGIN_NAME} DESTINATION "${QML_INSTALL_DIR}/${URI_PATH}")
install(FILES "${PROJECT_SOURCE_DIR}/qmlplugin/qmldir" DESTINATION "${QML_INSTALL_DIR}/${URI_PATH}")
