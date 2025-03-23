if(EnableQt5)
    qtquick_compiler_add_resources(MAINCOMPONENT_RCC_SOURCES maincomponent.qrc)
endif()
if(EnableQt6)
    qt_add_resources(MAINCOMPONENT_RCC_SOURCES maincomponent.qrc)
endif()

# Add LIB
add_library(${MAINCOMPONENT_LIB_NAME} SHARED
    maincomponentplugin.cpp
    ${MAINCOMPONENT_RCC_SOURCES}
)

target_link_libraries(${MAINCOMPONENT_LIB_NAME} PRIVATE
    ${LIB_NAME}
)

set_target_properties(${MAINCOMPONENT_LIB_NAME} PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${APPLOADER_PLUGINDUMP_OUTPUT_DIR}"
)
