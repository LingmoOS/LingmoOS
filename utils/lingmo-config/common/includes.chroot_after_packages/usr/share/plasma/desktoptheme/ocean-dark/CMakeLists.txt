configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/metadata.json.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
    @ONLY
)

install(
    FILES colors "${CMAKE_CURRENT_BINARY_DIR}/metadata.json"
          plasmarc
    DESTINATION ${PLASMA_DATA_INSTALL_DIR}/desktoptheme/ocean-dark
)

FILE(GLOB widgets widgets/*.svg*)
plasma_install_desktoptheme_svgs(ocean-dark SUBPATH widgets FILES ${widgets})

FILE(GLOB dialogs dialogs/*.svg*)
plasma_install_desktoptheme_svgs(ocean-dark SUBPATH dialogs FILES ${dialogs})

FILE(GLOB opaque_dialogs opaque/dialogs/*.svg*)
plasma_install_desktoptheme_svgs(ocean-dark SUBPATH opaque/dialogs FILES ${opaque_dialogs})

FILE(GLOB opaque_widgets opaque/widgets/*.svg*)
plasma_install_desktoptheme_svgs(ocean-dark SUBPATH opaque/widgets FILES ${opaque_widgets})

FILE(GLOB translucent_widgets translucent/widgets/*.svg*)
plasma_install_desktoptheme_svgs(ocean-dark SUBPATH translucent/widgets FILES ${translucent_widgets})

FILE(GLOB translucent_dialogs translucent/dialogs/*.svg*)
plasma_install_desktoptheme_svgs(ocean-dark SUBPATH translucent/dialogs FILES ${translucent_dialogs})

FILE(GLOB icons icons/*.svg*)
plasma_install_desktoptheme_svgs(ocean-dark SUBPATH icons FILES ${icons})

FILE(GLOB solid_widgets solid/widgets/*.svgz)
plasma_install_desktoptheme_svgs(ocean-dark SUBPATH solid/widgets FILES ${solid_widgets})

FILE(GLOB solid_dialogs solid/dialogs/*.svgz)
plasma_install_desktoptheme_svgs(ocean-dark SUBPATH solid/dialogs FILES ${solid_dialogs})
