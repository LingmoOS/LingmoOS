function (lingmo_ai_add_engine_plugin PROJECT_NAME ENGINE_TYPE)
    if (${ENGINE_TYPE} STREQUAL LARGE_MODEL)
        set(PLUGIN_PATH lingmo-ai/plugins/ai-engines/large-model)
    elseif (${ENGINE_TYPE} STREQUAL TRANDITIONNAL_MODEL)
        set(PLUGIN_PATH  lingmo-ai/plugins/ai-engines/traditional-model)
    elseif(${ENGINE_TYPE} STREQUAL VECTORDB_MODEL)
        set(PLUGIN_PATH  lingmo-ai/plugins/db-engines/vector-database)
    endif()

    add_library(${PROJECT_NAME} SHARED ${ARGN})
    target_link_libraries(${PROJECT_NAME} LingmoAiEngine::LingmoAiEngine)
    include(GNUInstallDirs)
    install(TARGETS ${PROJECT_NAME} DESTINATION
        "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}/${PLUGIN_PATH}")

endfunction()