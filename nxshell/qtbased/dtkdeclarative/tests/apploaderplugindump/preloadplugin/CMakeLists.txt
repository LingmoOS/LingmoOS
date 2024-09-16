if(EnableQt5)
    qtquick_compiler_add_resources(PRELOAD_RCC_SOURCES preload.qrc)
endif()
if(EnableQt6)
    qt_add_resources(PRELOAD_RCC_SOURCES preload.qrc)
endif()

add_library(${PRELOAD_LIB_NAME} SHARED
    preloadplugin.cpp
    ${PRELOAD_RCC_SOURCES}
)

target_link_libraries(${PRELOAD_LIB_NAME} PRIVATE
    ${LIB_NAME}
)

set_target_properties(${PRELOAD_LIB_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${APPLOADER_PLUGINDUMP_OUTPUT_DIR}"
)
