set(SOURCES "window_decorations.scss" "build_decorations_css.sh")
set(DECORATIONS_CSS_PATH "${PROJECT_BINARY_DIR}/kded/scss")

add_custom_command(
    OUTPUT "${DECORATIONS_CSS_PATH}/window_decorations.css"
    DEPENDS ${SOURCES}
    COMMAND ./build_decorations_css.sh "${DECORATIONS_CSS_PATH}"
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/kded/scss"
)
add_custom_target(BreezeGTKWindowDecorationsCSS ALL DEPENDS "${DECORATIONS_CSS_PATH}/window_decorations.css")

install(
    FILES "${DECORATIONS_CSS_PATH}/window_decorations.css"
    DESTINATION "${KDE_INSTALL_FULL_DATAROOTDIR}/themes/Breeze"
)
